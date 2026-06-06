#ifndef ZC_DEBUGGER_DEBUGGER_H_
#define ZC_DEBUGGER_DEBUGGER_H_

#include "imgui.h"
#include "base/expected.h"
#include "components/zasm/eval.h"
#include "components/zasm/pc.h"
#include "zc/debugger/vm.h"
#include "zc/ffscript.h"
#include "../third_party/ImGuiColorTextEdit/src/TextEditor.h"
#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct ValueStringOptions
{
	bool newlines = false;
	bool elide_arrays = false;
	bool exclude_deprecated = false;
	bool compact_fields = false;
	bool elide_zero_values = false;
};

struct FileNode
{
	std::string name;
	bool isFolder = false;
	const SourceFile* data = nullptr; // null if folder.
	std::map<std::string, FileNode> children;
};

struct Variable;

struct ConsoleMessage 
{
    std::string body;
    std::unique_ptr<StackTrace> stack_trace;
	std::unique_ptr<Variable> variable;
};

struct Breakpoint
{
	enum class Type
	{
		Error = -2,
		ScriptStart = -1,
		Normal = 0,
	};

	Type type = Type::Normal;
	const SourceFile* source_file;
	int line;
	pc_t pc;
	bool enabled = true;

	bool operator<(const Breakpoint& other) const
	{
		return pc < other.pc;
	}

	bool operator==(const Breakpoint& other) const
	{
		return pc == other.pc;
	}
};

struct Variable
{
	DebugValue value;
	std::string name;
	std::string value_str;
	ImU32 color;

	const DebugSymbol* symbol = nullptr;
	bool is_synthetic = false;
	bool is_expandable = false;
	bool children_populated = false;
	std::vector<Variable> children;
};

struct VariableGroup
{
	std::string label;
	std::vector<Variable> variables;
};

// Either displays the result of an expression, or an object defined by a value.
struct Watch
{
	std::string expression;
	std::string value_label;
	DebugValue value;
};

struct ActiveScript
{
	std::string name;
	ScriptEngineData* data;
	int sprite_id;
};

struct Debugger
{
	enum class State
	{
		Playing,
		Paused,
		PlayUntil,
		StepOver,
		StepInto,
		StepOut,
	};

	VM vm;
	ALLEGRO_DISPLAY* display;
	ALLEGRO_EVENT_QUEUE* queue;

	TextEditor text_editor;
	bool has_initialized_gui = false;

	FileNode root_node;
	ActiveScript* selected_script = nullptr;
	std::vector<ActiveScript> active_scripts;
	std::optional<StackTrace> current_stack_trace;
	const SourceFile* selected_source_file = nullptr;
	int selected_stack_frame_index = 0;
	const DebugScope* selected_scope = nullptr;
	int highlight_sprite_id = 0;
	std::deque<ConsoleMessage> console_logs;
	std::vector<std::string> console_command_history;
    int history_pos = -1; // -1 indicates "new expression" (bottom of history).
	bool console_scroll_to_bottom = false;
	std::vector<Breakpoint> breakpoints;
	std::vector<Breakpoint> breakpoints_deduped;
	bool break_on_new_script = false;
	bool break_on_error = false;
	bool breakpoints_dirty = true;
	std::vector<VariableGroup> variable_groups;
	std::vector<Watch> watches;
	std::vector<Variable> watch_variables;
	bool variables_dirty = false;
	std::map<const SourceFile*, std::vector<bool>> source_line_can_have_breakpoint_map;
	std::map<std::pair<std::string, int>, int> variable_name_to_imgui_id;
	int next_imgui_id = 0;

	State state = State::Playing;
	State target_state = State::Playing;
	const refInfo* last_ri = nullptr;
	const SourceFile* last_source_file = nullptr;
	std::chrono::steady_clock::time_point playing_last_update_time;
	int last_line = 0;
	int last_retsp = 0;
	pc_t pc_until = 0;

	Debugger();
	~Debugger();

	void Clear();
	void Init();
	void Load();
	void Save();
	void InitGui();

	// Breakpoints.
	void AddBreakpoint(const SourceFile* source_file, int line, pc_t pc, bool enabled = true);
	void RemoveBreakpoint(pc_t pc);
	void RemoveBreakpoints();
	bool HasBreakpoint(pc_t pc);

	// Watches.
	void AddWatchExpression(std::string expression);
	void AddWatchValue(DebugValue value, std::string label);
	void RemoveWatches();

	// Console messages.
	void AddConsoleMessageWithStackTrace(std::string message, StackTrace stack_trace);
	void AddConsoleMessage(std::string message);
	void AddConsoleDebugValue(std::string expression, DebugValue value);
	void AddToConsoleExpressionHistory(const std::string& cmd);

	// Flow control & State.
	void UpdateTextEditorBreakpoints();
	void SetSourceFile(const SourceFile* source_file);
	void SetSourceFileAndLine(const SourceFile* source_file, int line);
	void UpdateActiveScripts();
	void SetState(State new_state);
	void SetSelectedScriptIndex(int index);
	void SetSelectedStackFrameIndex(int index);
	const Variable* FindVariable(const std::string& name) const;

	// Variable inspection.
	void UpdateVariables();
	void FetchChildren(Variable& var);
	expected<DebugValue, std::string> Evaluate(std::string expression, bool suppress_engine_errors);

	// Stringify values.
	std::string ValueToStringSummary(DebugValue value);
	std::string ValueToStringFull(DebugValue value, ValueStringOptions opts = {});
	std::string ValueToStringTooltip(DebugValue value);

private:
	std::string ValueToStringHelper(DebugValue value, ValueStringOptions opts, int depth, std::set<int>& seen);
	Variable CreateVariableFromValue(const std::string& name, DebugValue value);
	void RenderVariableNode(Variable& var, std::vector<Variable*>& parents);
};

Debugger* zscript_debugger_open();
Debugger* zscript_debugger_get_if_open();
bool zscript_debugger_is_open();
void zscript_debugger_clear();
void zscript_debugger_init();
void zscript_debugger_update();
void zscript_debugger_exec(pc_t pc);
void zscript_debugger_close();

#endif
