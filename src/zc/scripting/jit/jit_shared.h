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
#include <memory>
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
	// ri->script_d - the current script's instance variables.
	int32_t* script_registers;
	int32_t* stack_base;
	uint32_t sp;
	pc_t pc;
	pc_t call_pc;
	// Used by the x64 backend to save the callee-saved registers it pins; the
	// a64 backend uses virtual registers and leaves this untouched.
	uint64_t saved_regs[4];
	// The native address to jump to on entry when resuming after a call or wait. The x64 backend
	// always uses this; the a64 backend resumes by either using this (for functions with many
	// resume points) or by comparing pc instead (for small functions).
	uintptr_t resume_address;
	// How the compiled function was entered. 0 = by the driver (pc/
	// resume_address describe where to resume). 1 = a direct native call from
	// another compiled function (a64 only): always starts at the function
	// start, and returns its exec result to the call site instead of the
	// driver - EXEC_RESULT_RETURN means a normal return, anything else is an
	// unwind the call site passes through untouched.
	int32_t entry_mode;
};

using JittedFunctionImpl = int (*)(JittedExecutionContext* ctx);

struct JittedFunction
{
	JittedFunctionImpl exec;
	pc_t id;
	std::map<pc_t, uintptr_t> pc_to_resume_address;
};

// The control flow and D-register liveness a function is compiled with.
struct JitFunctionAnalysis
{
	ZasmCFG cfg;
	ZasmLiveness liveness;
	std::vector<std::vector<pc_t>> block_predecessors;
};

struct JittedScript
{
	StructuredZasm structured_zasm;
	// What the liveness analysis knows about each function (by id), from analyzing the whole
	// script: what a call to it reads and writes, and what may be read after it returns.
	ZasmFunctionLivenessList function_liveness;
	// The D-registers live on entry to each function (by id), which the liveness of a function
	// that jumps or falls through to it depends on. With this and function_liveness, a
	// function's analysis can be built just before compiling it, rather than holding it for the
	// whole script.
	std::vector<uint8_t> function_live_in;
	// The analysis of the whole script. Only set if control enters some function somewhere other
	// than its start, which a function's own analysis can't account for. When set,
	// jit_analyze_function returns this for every function instead of building a per-function
	// analysis.
	std::unique_ptr<JitFunctionAnalysis> script_analysis;
	std::vector<pc_t> function_start_pcs;
	std::vector<JittedFunction> compiled_functions;
	std::deque<JittedFunction> pending_compiled_jit_functions;
	// Native entry per function for direct calls between compiled functions,
	// indexed by function id. jit_direct_not_compiled until the function is
	// compiled and committed; only non-yielding functions ever get a real
	// entry (a yield must unwind to the driver, which a native call frame
	// cannot survive). Call sites load the slot at runtime and call it
	// unconditionally - the stub turns the call into a driver-path call - so
	// this doubles as the hot-swap point when a function finishes compiling.
	std::unique_ptr<uintptr_t[]> direct_entry_table;
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

// The analysis to compile a function with: the script's if it has one, otherwise `storage`,
// filled in for just this function.
const JitFunctionAnalysis& jit_analyze_function(zasm_script* script, JittedScript* j_script, const ZasmFunction& fn, JitFunctionAnalysis& storage);

int32_t jit_direct_enter(JittedExecutionContext* ctx, int32_t callee_start_pc);
int32_t jit_direct_not_compiled(JittedExecutionContext* ctx);
void jit_direct_retstack_pop();

#endif
