#include "core/qst.h"
#include "base/util.h"
#include "base/zapp.h"
#include "test_runner/assert.h"
#include "test_runner/test_runner.h"
#include "components/zasm/debug_data.h"
#include "zc/debugger/debugger.h"
#include "zc/replay.h"
#include "zc/zelda.h"
#include <cstring>
#include <coroutine>
#include <exception>
#include <functional>

using namespace std::literals::string_literals;

std::function<void()> test_update;

struct TestTask;

struct TestTaskPromise
{
	// If this task is awaiting another task, we store the handle here
	// so we can forward the resume call to the leaf.
	std::coroutine_handle<> nested_task = nullptr;
	std::string failure_message;
	bool has_failed = false;

	TestTask get_return_object();

	std::suspend_always initial_suspend() { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }
	void return_void() {}
	void unhandled_exception() { std::terminate(); }
};

struct TestTask
{
	using promise_type = TestTaskPromise;
	std::coroutine_handle<promise_type> handle;

	TestTask(std::coroutine_handle<promise_type> h) : handle(h) {}

	// Disable copy, allow move (standard for RAII handles)
	TestTask(const TestTask&) = delete;
	TestTask(TestTask&& other) noexcept : handle(other.handle) { other.handle = nullptr; }
	~TestTask() { if (handle) handle.destroy(); }

	// --- 1. The Resume Logic (Recursive) ---
	// This is called by your Game Loop. It finds the active leaf task and resumes it.
	bool resume()
	{
		if (!handle || handle.done()) return false;

		// If we are awaiting a nested task (e.g. WaitForPause), resume THAT instead.
		if (handle.promise().nested_task)
		{
			// If the nested task is done after this resume, clear the pointer
			if (handle.promise().nested_task.done())
			{
				handle.promise().nested_task = nullptr;
				// We don't resume ourselves immediately; we wait for the next frame
				// or we could loop here. For a game loop, waiting is usually safer.
				return true;
			}

			// Recursive Resume: Only resume the nested task
			// We cast to our type to access the recursive .resume() logic if needed,
			// or just use raw resume() if we trust the chain. 
			// Ideally, we need to recurse if the nested task has ITS OWN nested task.
			// But since std::coroutine_handle<> is type-erased, we can't easily access the promise.
			// Simplified approach: Raw resume the direct child. 
			// (Note: To support deep nesting A->B->C, B must also forward to C. 
			//  This logic happens inside B's await_suspend below).
			handle.promise().nested_task.resume();

			// If it finished just now, clear it so we can run next frame
			if (handle.promise().nested_task.done())
				handle.promise().nested_task = nullptr;

			return true;
		}

		handle.resume();
		return !handle.done();
	}

	// --- 2. Make TestTask Awaitable (Enable 'co_await subTask') ---

	bool await_ready() { return !handle || handle.done(); }

	// When Parent awaits Child, this runs. 
	// We link Child to Parent so Parent knows to resume Child next time.
	void await_suspend(std::coroutine_handle<TestTaskPromise> parent) 
	{
		// Store our handle in the parent's promise
		parent.promise().nested_task = handle;
	}

	void await_resume() {}
};

// Implement deferred definition
inline TestTask TestTaskPromise::get_return_object() {
	return {std::coroutine_handle<TestTaskPromise>::from_promise(*this)};
}

// When a task awaits NextFrame, it yields execution completely.
// Because the parent (if any) is pointing to this task via 'nested_task',
// the resume chain stops here, returning control to the game loop.
struct NextFrame
{
	bool await_ready() { return false; }
	void await_suspend(std::coroutine_handle<>) {}
	void await_resume() {}
};

TestTask WaitFor(std::function<bool()> condition)
{
	while (!condition())
		co_await NextFrame{};
}

TestTask WaitForPause(Debugger* debugger)
{
	while (debugger->state != Debugger::State::Paused)
		co_await NextFrame{};
}

TestTask PlayAndWaitForPause(Debugger* debugger)
{
	debugger->SetState(Debugger::State::Playing);
	while (debugger->state != Debugger::State::Paused)
		co_await NextFrame{};
}

TestTask StepOver(Debugger* debugger)
{
	debugger->SetState(Debugger::State::StepOver);
	co_await WaitForPause(debugger);
}

TestTask StepInto(Debugger* debugger)
{
	debugger->SetState(Debugger::State::StepInto);
	co_await WaitForPause(debugger);
}

static void add_breakpoint(Debugger* debugger, std::string path, std::string content)
{
	const SourceFile* source_file = nullptr;
	for (auto& file : zasm_debug_data.source_files)
	{
		if (file.path.ends_with(path))
			source_file = &file;
	}

	auto lines = util::split(source_file->contents, "\n");

	int line = 0;
	for (int i = 0; i < lines.size(); i++)
	{
		if (lines[i].find(content) != std::string::npos)
		{
			line = i + 1;
			break;
		}
	}

	if (line == 0)
		throw std::runtime_error(fmt::format("Failed to set breakpoint at content: {}", content));

	auto pcs = zasm_debug_data.resolveAllPcsFromSourceLocation(source_file, line);
	for (pc_t pc : pcs)
		debugger->AddBreakpoint(source_file, line, pc);
	debugger->SetSourceFile(source_file);
}

static void select_script(Debugger* debugger, std::string name)
{
	for (int i = 0; i < debugger->active_scripts.size(); i++)
	{
		auto& active_script = debugger->active_scripts[i];
		if (active_script.name == name)
		{
			debugger->SetSelectedScriptIndex(i);
			return;
		}
	}

	throw std::runtime_error(fmt::format("Failed to select script: {}", name));
}

static void verify_variable(Debugger* debugger, std::string name, std::string expected_value)
{
	Variable* var = nullptr;
	for (auto& g : debugger->variable_groups)
	{
		for (auto& v : g.variables)
		{
			if (v.name == name)
			{
				var = &v;
				goto loopDone;
			}
		}
	}
	loopDone:

	if (!var)
		throw std::runtime_error(fmt::format("Assertion failed: could not find variable: {}", name));

	if (var->value_str != expected_value)
		throw std::runtime_error(fmt::format("Assertion failed: expected `{}` to equal `{}`, but got `{}`", name, expected_value, var->value_str));
}

static void verify_variable_not_present(Debugger* debugger, std::string name)
{
	Variable* var = nullptr;
	for (auto& g : debugger->variable_groups)
	{
		for (auto& v : g.variables)
		{
			if (v.name == name)
			{
				var = &v;
				goto loopDone;
			}
		}
	}
	loopDone:

	if (var)
		throw std::runtime_error(fmt::format("Assertion failed: expected to not find variable: {}", name));
}

static void verify_expression(Debugger* debugger, std::string expression, std::string expected_value)
{
	auto value = debugger->Evaluate(expression, false);
	if (!value)
		throw std::runtime_error(fmt::format("Assertion failed: `{}` is invalid. Error: {}", expression, value.error()));

	std::string actual = debugger->ValueToStringFull(*value);
	if (actual != expected_value)
		throw std::runtime_error(fmt::format("Assertion failed: expected `{}` to evaluate to `{}`, but got `{}`", expression, expected_value, actual));
}

static void verify_expression_invalid(Debugger* debugger, std::string expression, std::string expected_error)
{
	auto value = debugger->Evaluate(expression, false);
	if (value)
	{
		std::string actual = debugger->ValueToStringFull(*value);
		throw std::runtime_error(fmt::format("Assertion failed: expected `{}` to be invalid, but got '{}'", expression, actual));
	}

	std::string actual = value.error();
	if (actual != expected_error)
		throw std::runtime_error(fmt::format("Assertion failed: expected `{}` to error with `{}`, but got `{}`", expression, expected_error, actual));
}

static TestTask run_maths_replay_save_breakpoints_coroutine()
{
	auto* debugger = zscript_debugger_open();
	assertTrue(debugger);

	// Pause on first line.
	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "maths.zs", "bool firstTime = true;");
	co_await WaitForPause(debugger);

	zscript_debugger_close();
}

static TestTask run_maths_replay_coroutine()
{
	auto* debugger = zscript_debugger_open();
	assertTrue(debugger);

	// Should pause on first line from the last test, which saved breakpoints.
	assertSize(debugger->breakpoints, 1);
	co_await WaitForPause(debugger);

	verify_variable(debugger, "firstTime", "false"); // Not initialized yet.
	verify_variable(debugger, "x", "0.0000");
	verify_expression(debugger, "firstTime", "false");

	co_await StepOver(debugger);

	verify_variable(debugger, "firstTime", "true");
	verify_variable(debugger, "x", "0.0000");

	// Pause within for-loop.
	add_breakpoint(debugger, "maths.zs", "doMaths(firstTime, i);");
	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "i", "-10.0000");
	verify_variable(debugger, "firstTime", "true");

	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "i", "-9.0000");
	verify_variable(debugger, "firstTime", "true");

	co_await StepInto(debugger);

	verify_variable(debugger, "i", "-9.0000");
	verify_variable(debugger, "firstTime", "true");

	// Pause in maybePrint.
	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "maths.zs", "if (doPrint)");
	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "doPrint", "true");
	verify_variable(debugger, "str", "\"maths for: %d\\n\"");
	verify_variable(debugger, "value", "-9.0000");
	verify_variable_not_present(debugger, "v");

	co_await StepOver(debugger);
	co_await StepOver(debugger);

	verify_variable(debugger, "doPrint", "true");
	verify_variable(debugger, "str", "\"maths for: %d\\n\"");
	verify_variable(debugger, "value", "-9.0000");
	verify_variable(debugger, "v", "-9.0000");
	verify_expression(debugger, "str", "\"maths for: %d\\n\"");

	// Pause on Waitframe.
	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "maths.zs", "Waitframe();");
	co_await PlayAndWaitForPause(debugger);

	verify_variable_not_present(debugger, "i");
	verify_variable(debugger, "firstTime", "true");
	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "firstTime", "false");

	debugger->breakpoints.clear();
	debugger->SetState(Debugger::State::Playing);
}

static TestTask run_scopes_replay_coroutine()
{
	auto* debugger = zscript_debugger_open();
	assertTrue(debugger);

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// end of B_fn");
	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "GLOBAL_VAR", "3.0000");

	// Pause within first call to `new A::CL()`
	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "d->dosomething()");
	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "d", "DataBag {var1 = 1.0000, var2 = 2.0000}");
	verify_expression(debugger, "d->var1", "1.0000");
	verify_expression(debugger, "d->var2", "2.0000");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// end of DataBag::dosomething");
	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "this->var1", "1.0000");
	verify_expression(debugger, "this->var2", "2.0000");
	verify_expression(debugger, "var1", "1.0000");
	verify_expression(debugger, "var2", "2.0000");
	verify_expression(debugger, "v1", "1.0000");
	verify_expression(debugger, "v2", "2.0000");

	verify_expression(debugger, "var1 = 1337", "1337.0000");
	verify_expression(debugger, "var1", "1337.0000");
	verify_expression_invalid(debugger, "this = NULL", "Cannot assign to const variable: this");

	// Select second-top frame, then evaluate.
	debugger->SetSelectedStackFrameIndex(1);
	verify_expression(debugger, "this", "CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}");
	verify_expression(debugger, "A_var", "11.0000");
	verify_expression(debugger, "d->var1", "1337.0000");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// end of CL ctor");
	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "A_var", "11.0000");
	verify_variable(debugger, "this_ptr", "CL");
	verify_variable(debugger, "ptr", "CL");
	verify_variable(debugger, "CONST_GLOBAL_VAR", "1.0000");
	verify_variable_not_present(debugger, "this"); // hidden
	verify_expression(debugger, "ptr->arr", "{0.0000, 1.0000, 2.0000}");
	verify_expression(debugger, "ptr->arr_untyped", "{0, 10000, CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = ..., this_ptr = ...}}");
	verify_expression(debugger, "ptr", "CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}");
	verify_expression(debugger, "this->A_var", "11.0000");
	verify_expression(debugger, "this->this_ptr", "CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}");
	verify_expression(debugger, "CONST_GLOBAL_VAR", "1.0000");
	verify_expression(debugger, "Screen->Music", "null (-1)");
	verify_expression(debugger, "Audio->PanStyle", "1.0000");

	// Select a different script.
	debugger->SetSelectedScriptIndex(0);
	select_script(debugger, "TestRunner (DMAP)");
	verify_expression(debugger, "name", "\"scopes\"");
	select_script(debugger, "*scopes (GENERIC)");
	verify_expression_invalid(debugger, "name", "Unknown variable: name");
	select_script(debugger, "TestRunner (DMAP)");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "template_fn(1337, 1337)");
	co_await PlayAndWaitForPause(debugger);
	co_await StepInto(debugger);
	co_await StepOver(debugger);
	co_await StepOver(debugger);
	co_await StepOver(debugger);

	verify_variable(debugger, "double_arr", "Array[1]");
	verify_expression(debugger, "double_arr", "{{1337.0000}}");

	add_breakpoint(debugger, "scopes.zs", "template_fn(\"hello\", 1337)");
	co_await PlayAndWaitForPause(debugger);
	co_await StepInto(debugger);
	co_await StepOver(debugger);
	co_await StepOver(debugger);
	co_await StepOver(debugger);

	verify_variable(debugger, "double_arr", "Array[1]");
	verify_expression(debugger, "double_arr", "{{\"hello\"}}");

	add_breakpoint(debugger, "scopes.zs", "// end of trace_all_2");
	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "a", "Array[3]");
	verify_expression(debugger, "a", "{1.0000, 2.0000, 3.0000}");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// end of DebugArrays");
	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "arr1", "Array[2]");
	verify_variable(debugger, "arr2", "Array[2]");
	verify_variable(debugger, "arr3", "Array[8]");
	verify_variable(debugger, "arr4", "Array[8]");
	verify_variable(debugger, "arr5", "Array[2]");
	verify_variable(debugger, "arr6", "Array[4]");
	verify_variable(debugger, "arr7", "Array[2]");

	verify_expression(debugger, "arr1", "{1.0000, 2.0000}");
	verify_expression(debugger, "arr2", "{1.0000, 2.0000}");
	verify_expression(debugger, "arr3", "{1.0000, 1.0000, 2.0000, 1.0000, 1.0000, 2.0000, 1.0000, 1.0000}");
	verify_expression(debugger, "arr4", "{true, true, true, true, true, true, true, true}");
	verify_expression(debugger, "arr5", "{CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}, null}");
	verify_expression(debugger, "arr6", "{CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}, 1230000, {10000, 20000}, bitmap {Height = 32.0000, Width = 32.0000}}");
	verify_expression(debugger, "arr7", "{1.0000, 2.0000}");
	verify_expression(debugger, "Hero->Steps", "{1.0000, 1.0000, 2.0000, 1.0000, 1.0000, 2.0000, 1.0000, 1.0000}");

	verify_expression(debugger, "arr1[0]", "1.0000");
	verify_expression(debugger, "arr1[1]", "2.0000");
	verify_expression(debugger, "arr2[0]", "1.0000");
	verify_expression(debugger, "arr3[0]", "1.0000");
	verify_expression(debugger, "arr4[0]", "true");
	verify_expression(debugger, "arr5[0]", "CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}");
	verify_expression(debugger, "arr6[0]", "CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}");
	verify_expression(debugger, "arr7[0]", "1.0000");
	verify_expression(debugger, "Hero->Steps[0]", "1.0000");

	verify_expression(debugger, "arr1[-1]", "2.0000");
	verify_expression(debugger, "arr1[-2]", "1.0000");
	verify_expression_invalid(debugger, "arr1[-3]", "Invalid index for array");
	verify_expression_invalid(debugger, "arr1[2]", "Invalid index for array");
	verify_expression(debugger, "Hero->Steps[-3]", "2.0000");
	verify_expression_invalid(debugger, "Hero->Steps[-9]", "Invalid index for array");
	verify_expression(debugger, "arr1[1 + 0]", "2.0000");
	verify_expression(debugger, "arr1[0L]", "1.0000");
	verify_expression(debugger, "arr1[0 + 1L]", "1.0000"); // still index 0
	verify_expression(debugger, "arr1[1L]", "1.0000"); // 1 / 10000 = index 0
	verify_expression(debugger, "arr1[10000L]", "2.0000"); // index 1

	verify_expression(debugger, "arr6[0]->A_var", "11.0000");
	verify_expression(debugger, "arr6[0]->arr", "{0.0000, 1.0000, 2.0000}");
	verify_expression(debugger, "arr6[0]->arr[1]", "1.0000");
	verify_expression(debugger, "arr6[0]->arr_untyped", "{0, 10000, CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = ..., this_ptr = ...}}");
	verify_expression(debugger, "arr6[0]->arr_untyped[1]", "10000");
	verify_expression(debugger, "arr6[0]->arr_untyped[2]", "CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}");

	verify_expression(debugger, "arr1[1] = 100", "100.0000");
	verify_expression(debugger, "arr1", "{1.0000, 100.0000}");
	verify_expression(debugger, "arr1[0] = arr1[1] = 101", "101.0000");
	verify_expression(debugger, "arr1", "{101.0000, 101.0000}");
	verify_expression(debugger, "arr3[1] = 100", "100.0000");
	verify_expression(debugger, "arr3[1]", "100.0000");
	verify_expression(debugger, "Hero->Steps[1]", "100.0000");
	verify_expression(debugger, "arr2", "{1.0000, 2.0000}");

	verify_expression(debugger, "RefCount(arr1)", "1L");
	verify_expression(debugger, "RefCount(arr2)", "1L");
	verify_expression(debugger, "arr2 = arr1", "{101.0000, 101.0000}");
	verify_expression(debugger, "arr2", "{101.0000, 101.0000}");
	verify_expression(debugger, "RefCount(arr1)", "2L");

	verify_expression_invalid(debugger, "arr7 = arr2", "Cannot assign to const variable: arr7");

	verify_expression(debugger, "arr5[0]->A_var = 123", "123.0000");
	verify_expression(debugger, "arr5[0]->A_var", "123.0000");
	verify_expression(debugger, "arr5[0]->arr = arr2", "{101.0000, 101.0000}");
	verify_expression(debugger, "arr5[0]->arr", "{101.0000, 101.0000}");

	verify_expression(debugger, "arr2 = NULL", "0");
	verify_expression(debugger, "RefCount(arr1)", "1L");
	verify_expression(debugger, "arr6[0] = arr1", "{101.0000, 101.0000}");
	verify_expression(debugger, "RefCount(arr1)", "2L");
	verify_expression(debugger, "arr6[0] = NULL", "0");
	verify_expression(debugger, "RefCount(arr1)", "1L");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// end of DebugInternalObjects");
	co_await PlayAndWaitForPause(debugger);

	verify_variable(debugger, "b", "bitmap");

	// Typed stacks (`stack<int>` etc.) record as the plain `stack` class in the
	// debug data, and evaluate like any stack.
	verify_variable(debugger, "stk_i", "stack");
	verify_variable(debugger, "stk_b", "stack");
	verify_variable(debugger, "stk_u", "stack");
	verify_expression(debugger, "stk_i", "stack {Full = false, Size = 1L}");
	verify_expression(debugger, "stk_i->Size", "1L");
	verify_expression(debugger, "stk_b->Full", "false");
	verify_expression(debugger, "RefCount(stk_i)", "1L");

	verify_expression(debugger, "Enum_B", "(Enum) Enum_B");
	verify_expression(debugger, "MIDI_OVERWORLD", "-2.0000"); // anonymous enums are same as constants
	verify_expression(debugger, "BITDX_TRANS", "(BlitModeBitflags) BITDX_TRANS");
	verify_expression(debugger, "BITDX_TRANS | BITDX_PIVOT | BITDX_HFLIP", "(BlitModeBitflags) BITDX_TRANS | BITDX_PIVOT | BITDX_HFLIP");
	// ValueToStringSummary for fixed-point bitflags with unrecognized bits.
	{
		const DebugType* blitflags_type = nullptr;
		for (size_t i = 0; i < zasm_debug_data.scopes.size(); i++)
		{
			if (zasm_debug_data.scopes[i].tag == TAG_ENUM &&
				zasm_debug_data.scopes[i].name == "BlitModeBitflags")
			{
				for (auto& t : zasm_debug_data.types)
				{
					if (t.tag == TYPE_BITFLAGS && t.extra == (int32_t)i)
					{
						blitflags_type = &t;
						break;
					}
				}
				break;
			}
		}
		assertEqual(blitflags_type != nullptr, true);
		// BITDX_TRANS (1 int unit = 10000 raw) | unrecognized bit 0x10 (16 int units = 160000 raw) = 170000 raw.
		DebugValue val{170000, blitflags_type};
		assertEqual(debugger->ValueToStringSummary(val), "(BlitModeBitflags) BITDX_TRANS | 0x10"s);
	}
	verify_expression(debugger, "NUM_COMBO_POS", "176.0000");
	verify_expression(debugger, "OP_OPAQUE", "128.0000");
	verify_expression(debugger, "Hero->Step", "150.0000");
	verify_expression(debugger, "f->X", "10.0000");
	verify_expression(debugger, "f->EffectWidth", "16.0000");
	verify_expression(debugger, "f->Flags", "{true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}");
	verify_expression(debugger, "f->Flags[0]", "true");
	verify_expression(debugger, "n->X", "10.0000");
	verify_expression(debugger, "b", "bitmap {Height = 32.0000, Width = 32.0000}");
	verify_expression(debugger, "b->Width", "32.0000");

	// Engine objects in untyped arrays must resolve to their proper debug type.
	verify_expression(debugger, "arr_rng[0]", "randgen {}");

	verify_expression(debugger, "Hero->Step = 100", "100.0000");
	verify_expression(debugger, "Hero->Step", "100.0000");
	verify_expression(debugger, "f->X = 12.0000", "12.0000");
	verify_expression(debugger, "f->X", "12.0000");
	verify_expression(debugger, "GLOBAL_VAR", "1.0000");
	verify_expression(debugger, "GLOBAL_VAR = GLOBAL_VAR + 2", "3.0000");
	verify_expression(debugger, "GLOBAL_VAR", "3.0000");

	// User-defined functions.
	verify_expression_invalid(debugger, "DebugAdd()", "No matching function: DebugAdd"); // not enough parameters
	verify_expression(debugger, "DebugAdd(0)", "10.0000");
	verify_expression(debugger, "DebugAdd(1)", "11.0000");
	verify_expression(debugger, "DebugAdd(1, 1L)", "1.0101"); // a + b + 100L
	verify_expression(debugger, "enum_fn(Enum_B)", "(Enum) Enum_B");
	verify_expression(debugger, "A::DebugAddOne(10)", "11.0000");
	// TODO: optional params are currently not supported.
	// verify_expression(debugger, "A::DebugAddOne()", "2.0000");
	verify_expression(debugger, "DebugFunctionVarargs(1)", "1.0000");
	verify_expression(debugger, "DebugFunctionVarargs(1, 2, 3)", "6.0000");

	// 'new' expression.
	verify_expression(debugger, "new A::CL()", "CL {A_var = 11.0000, arr = {0.0000, 1.0000, 2.0000}, arr_untyped = {0, 10000, ...}, this_ptr = ...}");
	verify_expression(debugger, "new A::CL()->A_var", "11.0000");
	verify_expression(debugger, "new A::CL()->getVar()", "11.0000");
	verify_expression(debugger, "new bitmap(1, 1)->Width", "1.0000");
	verify_expression(debugger, "new A::DataBag()->var1", "1.0000");
	verify_expression(debugger, "new A::DataBag()->var2", "2.0000");
	verify_expression(debugger, "new A::DataBag()->sum(3)", "6.0000");
	verify_expression(debugger, "new A::DataBag()->sum(3, 4)", "10.0000");
	verify_expression(debugger, "new A::DataBag()->sum(3, 4, 5, 6)", "21.0000");

	// Internal functions (bindings).
	verify_expression(debugger, "ComboAt(0, 0)", "0.0000");
	verify_expression(debugger, "ComboAt(16, 0)", "1.0000");
	verify_expression(debugger, "Game->MaxEWeapons()", "255.0000");
	verify_expression(debugger, "Game->MaxEWeapons(11)", "");
	verify_expression(debugger, "Game->MaxEWeapons()", "11.0000");
	verify_expression(debugger, "Game->LoadTempScreen(10)", "null"); // invalid layer
	verify_expression(debugger, "Screen->GetExDoor(DIR_UP, 0)", "false");
	verify_expression(debugger, "Screen->SetExDoor(DIR_UP, 0, true)", "");
	verify_expression(debugger, "Screen->GetExDoor(DIR_UP, 0)", "true");
	verify_expression(debugger, "Screen->CreateNPC(NPC_GEL) > 0", "true");
	verify_expression(debugger, "Screen->CreateNPC(NPC_GEL)->DeathSprite", "23.0000");
	verify_expression(debugger, "Screen->CreateNPC(NPC_GEL)->isDead()", "false");
	verify_expression(debugger, "Screen->CreateNPC(NPC_GEL)->isValid()", "true");
	verify_expression(debugger, "Game->LoadMapData(2, 1)->ComboD[0]", "156.0000");
	verify_expression(debugger, "SizeOfArray(Game->LoadMapData(2, 1)->ComboD)", "176.0000");
	verify_expression(debugger, "Game->LoadMapData(2, 1)->GetExDoor(DIR_UP, 0)", "false");
	verify_expression(debugger, "Game->LoadMapData(2, 1)->SetExDoor(DIR_UP, 0, true)", "");
	verify_expression(debugger, "Game->LoadMapData(2, 1)->GetExDoor(DIR_UP, 0)", "true");
	verify_expression_invalid(debugger, "Waitframe()", "Cannot call Waitframe for expressions evaluated in the debugger");
	verify_expression_invalid(debugger, "DebugFunctionCallsWaitframe1()", "Cannot call Waitframe for expressions evaluated in the debugger");
	verify_expression_invalid(debugger, "DebugFunctionInfiniteLoop(true)", "Detected possible infinite loop, aborting function call");

	/*
	int DebugFrames1(int a, int b)
	{
		int c = a + b; // start of DebugFrames1
		int d = DebugFrames2(a + 1, b + 1);
		return c + d; // end of DebugFrames1
	}

	int DebugFrames2(int a, int b)
	{
		int c = a * b;
		return c; // end of DebugFrames2
	}
	*/

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// start of DebugFrames1");
	add_breakpoint(debugger, "scopes.zs", "// end of DebugFrames1");
	add_breakpoint(debugger, "scopes.zs", "// end of DebugFrames2");
	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "a", "2.0000");
	verify_expression(debugger, "b", "5.0000");

	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "a", "3.0000");
	verify_expression(debugger, "b", "6.0000");
	verify_expression(debugger, "c", "18.0000");

	// Select second-top frame, then evaluate.
	debugger->SetSelectedStackFrameIndex(1);
	verify_expression(debugger, "a", "2.0000");
	verify_expression(debugger, "b", "5.0000");
	verify_expression(debugger, "c", "7.0000");

	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "a", "2.0000");
	verify_expression(debugger, "b", "5.0000");
	verify_expression(debugger, "d", "18.0000");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// end of DebugFunctionDefaultCtor2");
	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "c[0]", "DebugClassDefaultCtor {data = 10.0000}");
	verify_expression(debugger, "c[0]->data", "10.0000");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// Create CL");
	co_await PlayAndWaitForPause(debugger);

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "// end of CL dtor");
	co_await PlayAndWaitForPause(debugger);

	verify_expression(debugger, "a", "11.0000");

	debugger->RemoveBreakpoints();
	add_breakpoint(debugger, "scopes.zs", "Test::End()");
	co_await PlayAndWaitForPause(debugger);

	// Separator rules: '.' for script/class members, '::' for namespaces.
	verify_expression(debugger, "scopes.SCRIPT_SCOPED_GLOBAL", "123.0000");
	verify_expression(debugger, "scopes.scriptFunction()", "100.0000");
	verify_expression(debugger, "A::cl", "null"); // never set
	verify_expression_invalid(debugger, "scopes::SCRIPT_SCOPED_GLOBAL", "Unknown variable: scopes::SCRIPT_SCOPED_GLOBAL");
	verify_expression_invalid(debugger, "scopes::scriptFunction()", "No matching function: scopes::scriptFunction");
	verify_expression_invalid(debugger, "A.cl", "Unknown variable: A.cl");
	verify_expression(debugger, "A::DataBag.StaticClassFunction()", "123.0000");
	verify_expression(debugger, "A::DataBag::StaticClassFunction()", "123.0000");

	// Functions that return a compile-time constant are optimized as nil functions and inlined (see
	// ReturnVisitor::analyzeFunctionInternals). Ensure the debugger can still call it.
	verify_expression(debugger, "returnOne()", "1.0000");

	verify_variable(debugger, "this", "genericdata"); // not hidden
	verify_expression(debugger, "this", "genericdata {Data = {}, DataSize = 0.0000, EventListen = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false}, ExitState = {false, false, false, false, false}, InitD = {0, 0, 0, 0, 0, 0, 0, 0}, ReloadState = {false, false, false, false, false}, Running = true}");
	verify_expression(debugger, "this->Running", "true");
	verify_expression_invalid(debugger, "this = NULL", "Cannot assign to const variable: this");

	debugger->breakpoints.clear();
	debugger->SetState(Debugger::State::Playing);
}

static void TEST(std::string name, TestResults& tr, std::function<bool()> cb)
{
	try
	{
		if (!cb())
			tr.failed++;
	}
	catch (const std::exception& e)
	{
		fmt::println("[{}] error: {}", name, e.what());
		tr.failed++;
	}

	test_update = nullptr;
	tr.total++;
}

TestResults test_debugger([[maybe_unused]] bool verbose)
{
	static TestResults tr{};

	// TODO: investigate why test fails in CI.
#ifdef _WIN32
	if (is_ci())
		return tr;
#endif

	int test_zc_arg = zapp_check_switch("-test-zc", {"test_dir"});
	CHECK(test_zc_arg > 0);
	static std::string test_dir = zapp_get_arg_string(test_zc_arg + 1);

	// TODO: this is very brittle.
	// Better approach: make a global "features" struct that is set on startup based on CLI/cfg, and
	// then here just override the settings directly rather than replace CLI args.
	// JIT isn't really utilized, but this ensures that the script engine falls back to interpreted mode
	// when the debugger is open.
	static std::vector<const char*> test_args = {"", "-test-zc", test_dir.data(), "-optimize-zasm", "-jit", "-debugger"};
	zapp_replace_args(test_args.size(), const_cast<char**>(test_args.data()));

	TEST("maths.zplay (save breakpoints)", tr, [](){
		TestTask current_test = run_maths_replay_save_breakpoints_coroutine();
		bool success = false;

		test_update = [&](){
			if (current_test.resume())
				return;

			auto& promise = current_test.handle.promise();
			if (promise.has_failed)
				fmt::println("error: {}", promise.failure_message);
			else
				success = true;

			test_update = nullptr;
		};

		load_replay_file_deferred(ReplayMode::Replay, test_dir + "/replays/playground/maths.zplay");
		init_and_run_main_zplayer_loop();

		assertTrue(success);
		return success;
	});

	TEST("maths.zplay", tr, [](){
		TestTask current_test = run_maths_replay_coroutine();
		bool success = false;

		test_update = [&](){
			if (current_test.resume())
				return;

			auto& promise = current_test.handle.promise();
			if (promise.has_failed)
				fmt::println("error: {}", promise.failure_message);
			else
				success = true;

			test_update = nullptr;
		};

		load_replay_file_deferred(ReplayMode::Replay, test_dir + "/replays/playground/maths.zplay");
		init_and_run_main_zplayer_loop();

		assertTrue(success);
		return success;
	});

	TEST("scopes.zplay", tr, [](){
		TestTask current_test = run_scopes_replay_coroutine();
		bool success = false;

		test_update = [&](){
			if (current_test.resume())
				return;

			auto& promise = current_test.handle.promise();
			if (promise.has_failed)
				fmt::println("error: {}", promise.failure_message);
			else
				success = true;

			test_update = nullptr;
		};

		load_replay_file_deferred(ReplayMode::Replay, test_dir + "/replays/playground/auto_scopes.zplay");
		init_and_run_main_zplayer_loop();

		assertTrue(success);
		return success;
	});

	return tr;
}
