#include "zc/debugger/debugger.h"

#include "base/expected.h"
#include "zalleg/new_menu.h"
#include "core/qst.h"
#include "base/util.h"
#include "base/zapp.h"
#include "core/zdefs.h"
#include "nonstd/expected.h"
#include "user_object.h"
#include "zalleg/zalleg.h"
#include "components/zasm/debug_data.h"
#include "components/zasm/eval.h"
#include "components/zasm/pc.h"
#include "zc/debugger/debugger_gui.h"
#include "zc/ffscript.h"
#include "zconfig.h"
#include "imgui.h"
#include "imgui_impl_allegro5.h"
#include "../third_party/ImGuiColorTextEdit/src/TextEditor.h"
#include <chrono>
#include <deque>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <vector>
#include <fmt/ranges.h>

namespace {

std::unique_ptr<Debugger> debugger;

int GetClassNullValue(std::string_view class_name)
{
	// Singleton globals have no @zasm_ref and use raw_value=0 as a valid instance; -1 is their null.
	if (class_name == "musicdata") return -1;
	if (class_name == "Audio")     return -1;
	if (class_name == "Debug")     return -1;
	if (class_name == "FileSystem") return -1;
	if (class_name == "Game")      return -1;
	if (class_name == "Graphics")  return -1;
	if (class_name == "hero")      return -1;
	if (class_name == "Input")     return -1;
	if (class_name == "Region")    return -1;
	if (class_name == "screendata") return -1;
	if (class_name == "Text")      return -1;
	if (class_name == "Viewport")  return -1;
	if (class_name == "ZInfo")     return -1;

	return 0;
}

bool IsNull(int raw_value, std::string_view class_name)
{
	// Fast path. Must check all values in GetClassNullValue.
	if (raw_value != 0 && raw_value != -1)
		return false;

	return raw_value == GetClassNullValue(class_name);
}

ImU32 GetColorForType(const TextEditor::Palette& palette, const DebugType* type)
{
	if (type->isString(zasm_debug_data))
		return palette[(int)TextEditor::PaletteIndex::String];

	return palette[(int)TextEditor::PaletteIndex::Keyword];
}

std::string GetScriptTypeName(ScriptType type)
{
	switch (type)
	{
		case ScriptType::Global:
			return "GLOBAL";
		case ScriptType::FFC:
			return "FFC";
		case ScriptType::Screen:
			return "SCREEN";
		case ScriptType::Hero:
			return "HERO";
		case ScriptType::Item:
			return "ITEMDATA";
		case ScriptType::Lwpn:
			return "LWEAPON";
		case ScriptType::NPC:
			return "NPC";
		case ScriptType::Ewpn:
			return "EWEAPON";
		case ScriptType::DMap:
			return "DMAP";
		case ScriptType::ItemSprite:
			return "ITEMSPRITE";
		case ScriptType::Combo:
			return "COMBODATA";
		case ScriptType::Generic: case ScriptType::GenericFrozen:
			return "GENERIC";
		case ScriptType::EngineSubscreen:
			return "SUBSCREEN";
		case ScriptType::None:
		default:
			return "UNKNOWN";
	}
}

} // end namespace

Debugger::Debugger()
{
	Init();
	InitGui();
}

Debugger::~Debugger()
{
	if (is_exiting())
		return;

	ImGui_ImplAllegro5_Shutdown();
	ImGui::DestroyContext();
	al_destroy_event_queue(queue);
	queue = nullptr;
	al_destroy_display(display);
	display = nullptr;
}

void Debugger::Clear()
{
	selected_script = nullptr;
	active_scripts.clear();
	selected_stack_frame_index = 0;
	vm.current_data = nullptr;
	vm.current_frame_index = 0;
	vm.suppress_errors_in_sandbox = true;
	current_stack_trace = std::nullopt;
	selected_scope = nullptr;
	breakpoints.clear();
	console_logs.clear();
	variable_groups.clear();
	watches.clear();
	watch_variables.clear();
	source_line_can_have_breakpoint_map.clear();
	root_node = {};
	SetSourceFile(nullptr);
}

void Debugger::Init()
{
	selected_script = nullptr;
	active_scripts.clear();
	selected_stack_frame_index = 0;
	vm.current_data = nullptr;
	vm.current_frame_index = 0;
	vm.suppress_errors_in_sandbox = true;
	current_stack_trace = std::nullopt;
	selected_scope = nullptr;
	RemoveBreakpoints();
	console_logs.clear();
	if (!zasm_debug_data.exists())
		AddConsoleMessage("NOTE: This quest was made before debug data. For the debugger, only the console is enabled.");
	variable_groups.clear();
	watches.clear();
	watch_variables.clear();
	source_line_can_have_breakpoint_map.clear();
	root_node = {};
	if (zasm_debug_data.exists())
		SetSourceFile(&zasm_debug_data.source_files[0]);
	Load();
}

void Debugger::Load()
{
	std::string qst_cfg_header = qst_cfg_header_from_path(qstpath);

	std::string active_file_path = zc_get_config(qst_cfg_header.c_str(), "debugger_active_file", "");
	if (auto source_file = zasm_debug_data.getSourceFile(active_file_path))
		SetSourceFile(source_file);

	std::string breakpoints_serialized = zc_get_config(qst_cfg_header.c_str(), "debugger_breakpoints", "");
	auto breakpoints_serialized_parts = util::split(breakpoints_serialized, ";");
	for (auto& breakpoint_serialized : breakpoints_serialized_parts)
	{
		auto components = util::split(breakpoint_serialized, ",");
		if (components.size() != 4)
			continue;

		std::string& type_str = components[0];
		std::string& enabled_str = components[1];
		std::string& path = components[2];
		std::string& line_str = components[3];

		if (type_str != "0")
			continue;

		const SourceFile* source_file = zasm_debug_data.getSourceFile(path);
		if (!source_file)
			continue;

		int line = std::stoi(line_str);
		bool enabled = enabled_str == "1";

		auto pcs = zasm_debug_data.resolveAllPcsFromSourceLocation(source_file, line);
		for (pc_t pc : pcs)
			AddBreakpoint(source_file, line, pc, enabled);
	}

	std::string watches_serialized = zc_get_config(qst_cfg_header.c_str(), "debugger_watches", "");
	auto watches_serialized_parts = util::split(watches_serialized, ";");
	for (auto& watch_serialized : watches_serialized_parts)
	{
		auto components = util::split(watch_serialized, ",");
		if (components.size() != 2)
			continue;

		std::string& type_str = components[0];
		std::string& expression = components[1];

		if (type_str != "0")
			continue;

		AddWatchExpression(expression);
	}
}

void Debugger::Save()
{
	std::string qst_cfg_header = qst_cfg_header_from_path(qstpath);

	if (selected_source_file)
		zc_set_config(qst_cfg_header.c_str(), "debugger_active_file", selected_source_file->path.c_str());

	std::vector<std::string> breakpoints_serialized_parts;
	for (auto& breakpoint : breakpoints_deduped)
	{
		int type = (int)breakpoint.type;
		if (type < 0)
			continue;

		int enabled = breakpoint.enabled ? 1 : 0;
		breakpoints_serialized_parts.push_back(fmt::format("{},{},{},{}", type, enabled, breakpoint.source_file->path, breakpoint.line));
	}

	std::string breakpoints_serialized = fmt::format("{}", fmt::join(breakpoints_serialized_parts, ";"));
	zc_set_config(qst_cfg_header.c_str(), "debugger_breakpoints", breakpoints_serialized.c_str());

	std::vector<std::string> watches_serialized_parts;
	for (auto& watch : watches)
	{
		if (!watch.expression.empty())
		{
			int type = 0;
			watches_serialized_parts.push_back(fmt::format("{},{}", type, watch.expression));
		}
	}

	std::string watches_serialized = fmt::format("{}", fmt::join(watches_serialized_parts, ";"));
	zc_set_config(qst_cfg_header.c_str(), "debugger_watches", watches_serialized.c_str());
}

void Debugger::AddBreakpoint(const SourceFile* source_file, int line, pc_t pc, bool enabled)
{
	Breakpoint new_bp = { Breakpoint::Type::Normal, source_file, line, pc };

	// std::lower_bound performs a binary search to find the first element 
	// that does not compare less than new_bp (i.e., the correct slot).
	auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), new_bp);

	// Check if it already exists to avoid duplicates.
	if (it != breakpoints.end() && it->pc == pc)
	{
		if (it->enabled != enabled)
		{
			it->enabled = enabled;
			breakpoints_dirty = true;
		}
		return; // Breakpoint already exists at this PC
	}

	new_bp.enabled = enabled;
	breakpoints.insert(it, new_bp);

	if (source_file == selected_source_file)
		text_editor.AddBreakpoint(line);

	breakpoints_dirty = true;
}

void Debugger::RemoveBreakpoint(pc_t pc)
{
	Breakpoint target = { Breakpoint::Type::Normal, nullptr, 0, pc };
	auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), target);
	if (it != breakpoints.end() && it->pc == pc)
	{
		if (it->source_file == selected_source_file)
			text_editor.RemoveBreakpoint(it->line);
		breakpoints.erase(it);
		breakpoints_dirty = true;
	}
}

void Debugger::RemoveBreakpoints()
{
	breakpoints.clear();
	breakpoints_deduped.clear();
	text_editor.SetBreakpoints({});
	breakpoints_dirty = true;
}

bool Debugger::HasBreakpoint(pc_t pc)
{
	Breakpoint target = {Breakpoint::Type::Normal, nullptr, 0, pc};
	auto it = std::lower_bound(breakpoints.begin(), breakpoints.end(), target);
	if (it != breakpoints.end() && !(target < *it))
		return it->enabled;
	return false;
}

void Debugger::AddWatchExpression(std::string expression)
{
	watches.emplace_back(expression);
	variables_dirty = true;
}

void Debugger::AddWatchValue(DebugValue value, std::string label)
{
	watches.emplace_back("", label, value);
	variables_dirty = true;
}

void Debugger::RemoveWatches()
{
	watches.clear();
	variables_dirty = true;
}

void Debugger::AddConsoleMessageWithStackTrace(std::string message, StackTrace stack_trace)
{
	if (!message.empty() && message.back() == '\n')
		message.pop_back();
	if (message.empty())
		return;

	console_logs.push_back(ConsoleMessage{
		.body = std::move(message),
		.stack_trace = std::make_unique<StackTrace>(std::move(stack_trace)),
	});

	while (console_logs.size() > 20000)
		console_logs.pop_front();

	console_scroll_to_bottom = true;
}

void Debugger::AddConsoleMessage(std::string message)
{
	if (!message.empty() && message.back() == '\n')
		message.pop_back();
	if (message.empty())
		return;

	console_logs.push_back(ConsoleMessage{
		.body = std::move(message),
	});

	while (console_logs.size() > 20000)
		console_logs.pop_front();

	console_scroll_to_bottom = true;
}

void Debugger::AddConsoleDebugValue(std::string expression, DebugValue value)
{
    auto var = CreateVariableFromValue(expression, value);

    console_logs.push_back(ConsoleMessage{
        .body = ValueToStringFull(value, {.newlines = true}),
        .variable = std::make_unique<Variable>(std::move(var)),
    });

    while (console_logs.size() > 20000)
		console_logs.pop_front();

    console_scroll_to_bottom = true;
}

void Debugger::AddToConsoleExpressionHistory(const std::string& cmd) 
{
	if (cmd.empty()) return;

	// Don't add duplicates of the very last expression.
	if (!console_command_history.empty() && console_command_history.back() == cmd) 
		return;

	console_command_history.push_back(cmd);

	if (console_command_history.size() > 1000) 
		console_command_history.erase(console_command_history.begin());

	// Reset position to the bottom.
	history_pos = -1; 
}

void Debugger::UpdateTextEditorBreakpoints()
{
	TextEditor::Breakpoints bps;
	for (auto& breakpoint : breakpoints)
	{
		if (breakpoint.source_file == selected_source_file)
		{
			auto& bp = bps[breakpoint.line] = {};
			bp.mLine = breakpoint.line;
			bp.mEnabled = breakpoint.enabled;
		}
	}
	text_editor.SetBreakpoints(std::move(bps));
}

void Debugger::SetSourceFile(const SourceFile* source_file)
{
	if (!source_file || selected_source_file == source_file)
		return;

	selected_source_file = source_file;
	text_editor.SetText(source_file ? source_file->contents : "");

	UpdateTextEditorBreakpoints();

	std::vector<bool>* executable_lines = nullptr;
	if (auto r = source_line_can_have_breakpoint_map.find(source_file); r != source_line_can_have_breakpoint_map.end())
	{
		executable_lines = &r->second;
	}
	else
	{
		auto& new_lines = source_line_can_have_breakpoint_map[source_file];
		new_lines.resize(text_editor.GetTotalLines());
		executable_lines = &new_lines;

		int source_file_index = zasm_debug_data.getSourceFileIndex(source_file);
		auto& line_table = zasm_debug_data.getLineTable();
		bool first = true;
		for (auto& it : line_table)
		{
			if (it.file_index == source_file_index)
			{
				new_lines[it.line_number - 1] = true;
				// Prevent function declarations (which is what the prologue maps to) from being a
				// breakpoint target.
				if (it.is_prologue_end && !first)
					new_lines[(&it-1)->line_number - 1] = false;
			}
			first = false;
		}
	}
	text_editor.SetExecutableLines(*executable_lines);
}

void Debugger::SetSourceFileAndLine(const SourceFile* source_file, int line)
{
	SetSourceFile(source_file);
	text_editor.SetCursorPosition({line - 1, 0});
	ImGui::SetWindowFocus("EditorPane");
}

void Debugger::UpdateActiveScripts()
{
	size_t idx = 0;

	for (auto& named_data : active_object_dtor_script_datas)
	{
		if (idx >= active_scripts.size())
			active_scripts.emplace_back();

		ActiveScript& script = active_scripts[idx];
		script.data = named_data->data.get();
		script.name = named_data->name;
		script.sprite_id = 0;

		idx++;
	}

	for (auto& data : scriptEngineDatas | std::views::values)
	{
		if (!data.initialized)
			continue;

		if (idx >= active_scripts.size())
			active_scripts.emplace_back();

		ActiveScript& script = active_scripts[idx];
		script.data = &data;
		script.name.clear();
		if (auto s = get_own_sprite(&data.ref, data.script_type))
			script.sprite_id = s->getUID();
		else
			script.sprite_id = 0;

		// Write directly into the existing buffer.
		fmt::format_to(std::back_inserter(script.name), "{} ({})",
			FFCore.GetScriptName(data.script_type, data.script_num),
			GetScriptTypeName(data.script_type));

		idx++;
	}

	active_scripts.resize(idx);
	selected_script = nullptr;
}

void Debugger::SetState(State new_state)
{
	extern refInfo *ri;
	extern script_data* curscript;

	if (!curscript || !ri)
		return;

	state = new_state;
	target_state = new_state;
	last_ri = nullptr;
	last_source_file = nullptr;
	current_stack_trace = std::nullopt;
	last_line = 0;
	last_retsp = 0;
	selected_scope = nullptr;
	selected_stack_frame_index = 0;
	vm.current_frame_index = 0;
	playing_last_update_time = {};

	if (state == State::Playing)
		return;

	current_stack_trace = FFCore.create_stack_trace(ri);

	// TODO: it'd be cool to focus the debugger display window, but not possible w/ allegro.

	if (current_stack_trace)
	{
		last_ri = ri;
		last_source_file = current_stack_trace->frames.front().source_file;
		last_line = current_stack_trace->frames.front().line;
		last_retsp = ri->retsp;
		if (state == State::Paused)
			SetSourceFileAndLine(last_source_file, last_line);
	}
	else if (state != State::Paused)
	{
		state = State::Playing;
	}

	if (state != State::Paused)
		return;

	UpdateActiveScripts();

	selected_script = nullptr;
	vm.current_data = nullptr;
	if (state != State::Playing)
	{
		for (auto& active_script : active_scripts)
		{
			if (&active_script.data->ref == ri)
			{
				active_script.name = "*" + active_script.name;
				selected_script = &active_script;
				vm.current_data = selected_script->data;
				break;
			}
		}
	}

	const DebugScope* scope = zasm_debug_data.resolveScope(ri->pc);

	const DebugScope* fn_scope = scope;
	while (fn_scope && fn_scope->tag != TAG_FUNCTION)
	{
		if (fn_scope->parent_index != -1)
			fn_scope = &zasm_debug_data.scopes[fn_scope->parent_index];
		else
			fn_scope = nullptr;
	}

	// If paused at the first instruction of a function, temporarily resume until the prologue
	// finishes. Otherwise parameters will not resolve correctly.
	if (fn_scope && ri->pc == fn_scope->start_pc)
	{
		pc_t prologue_end = zasm_debug_data.findFunctionPrologueEnd(fn_scope);
		if (ri->pc != prologue_end)
		{
			state = target_state = State::PlayUntil;
			pc_until = prologue_end;
			return;
		}
	}

	selected_scope = scope;
	UpdateVariables();
}

void Debugger::SetSelectedScriptIndex(int index)
{
	selected_script = &active_scripts[index];
	current_stack_trace = FFCore.create_stack_trace(&selected_script->data->ref);

	pc_t pc = current_stack_trace->frames[0].pc;
	selected_scope = zasm_debug_data.resolveScope(pc);
	vm.current_data = selected_script->data;
	vm.current_frame_index = 0;

	if (current_stack_trace)
		debugger->SetSourceFileAndLine(current_stack_trace->frames.front().source_file, current_stack_trace->frames.front().line);
	UpdateVariables();
}

void Debugger::SetSelectedStackFrameIndex(int index)
{
	selected_stack_frame_index = index;

	pc_t pc = current_stack_trace->frames[index].pc;
	selected_scope = zasm_debug_data.resolveScope(pc);
	vm.current_frame_index = index;
	UpdateVariables();
}

const Variable* Debugger::FindVariable(const std::string& name) const
{
	for (auto& [_, variables] : variable_groups)
	{
		for (auto& var : variables)
			if (var.name == name) return &var;
	}

	return nullptr;
}

void Debugger::UpdateVariables()
{
	auto CreateVariable = [&](DebugValue value, std::string name){
		// Discard const-ness.
		value.type = value.type->asNonConst(zasm_debug_data);
		std::string value_str = ValueToStringSummary(value);

		bool expandable = false;
		if (value.type->isArray(zasm_debug_data) && value.type->extra != TYPE_CHAR32)
		{
			expandable = value.raw_value != 0;
		}
		else if (value.type->isClass(zasm_debug_data))
		{
			expandable = !IsNull(value.raw_value, zasm_debug_data.getTypeName(value.type));
		}

		ImU32 color = GetColorForType(text_editor.GetPalette(), value.type);
		Variable var(value, name, value_str, color);
		var.is_expandable = expandable;
		return var;
	};

	variable_groups.clear();

	variable_groups.reserve(4);
	auto& local_group = variable_groups.emplace_back("Local");
	auto& class_group = variable_groups.emplace_back("Class");
	auto& script_group = variable_groups.emplace_back("Script");
	auto& global_group = variable_groups.emplace_back("Global");

	const DebugScope* scope = selected_scope;
	while (scope)
	{
		ExpressionEvaluator eval{zasm_debug_data, scope, vm};
		auto symbols = zasm_debug_data.getChildSymbols(scope);

		for (auto symbol : symbols)
		{
			if (symbol->flags & SYM_FLAG_HIDDEN)
				continue;

			DebugValue value = eval.readSymbol(symbol);
			Variable var = CreateVariable(value, symbol->name);
			var.symbol = symbol;

			if (symbol->storage == LOC_GLOBAL)
				var.name = zasm_debug_data.getFullSymbolName(symbol);

			if (scope->tag == TAG_CLASS)
				class_group.variables.push_back(var);
			else if (scope->tag == TAG_ROOT || scope->tag == TAG_FILE || scope->tag == TAG_NAMESPACE)
				global_group.variables.push_back(var);
			else if (scope->tag == TAG_SCRIPT)
				script_group.variables.push_back(var);
			else
				local_group.variables.push_back(var);
		}

		if (scope->parent_index == -1)
			break;

		if (scope->tag == TAG_FILE)
			break;

		scope = &zasm_debug_data.scopes[scope->parent_index];
	}

	watch_variables.clear();
	for (auto& watch : watches)
	{
		if (watch.expression.empty())
		{
			Variable var = CreateVariable(watch.value, watch.value_label);
			watch_variables.push_back(var);
		}
		else if (auto v = Evaluate(watch.expression, true))
		{
			Variable var = CreateVariable(v.value(), watch.expression);
			watch_variables.push_back(var);
		}
		else
		{
			DebugValue value{0, &BasicTypes[TYPE_VOID]};
			ImU32 color = (ImU32)text_editor.GetPalette()[(int)TextEditor::PaletteIndex::ErrorMarker];
			watch_variables.emplace_back(value, watch.expression, "not available", color);
		}
	}
}

void Debugger::FetchChildren(Variable& var)
{
	if (var.children_populated)
		return;

	var.children_populated = true;

	if (var.value.type->tag == TYPE_ARRAY)
	{
		auto* array = checkArray(var.value.raw_value, true);
		if (!array) return;

		if (auto values = vm.readArray(DebugValue{var.value.raw_value, var.value.type}))
		{
			for (int i = 0; i < values->size(); i++)
			{
				DebugValue value = values->at(i);
				std::string val_str = ValueToStringSummary(value);
				int elem_val = value.raw_value;
				const DebugType* elem_type = value.type->asNonConst(zasm_debug_data);
				bool is_expandable = (elem_type->tag == TYPE_ARRAY && elem_type->extra != TYPE_CHAR32) 
								|| (elem_type->tag == TYPE_CLASS && elem_val != 0);

				Variable child(value, fmt::format("[{}]", i), val_str, var.color);
				child.is_expandable = is_expandable;

				var.children.push_back(child);
			}
		}
	}
	else if (var.value.type->tag == TYPE_CLASS)
	{
		const DebugScope* class_scope = &zasm_debug_data.scopes[var.value.type->extra];

		if (IsNull(var.value.raw_value, class_scope->name))
			return;

		if (class_scope->inheritance_index != -1)
		{
			const DebugScope* parent_class_scope = &zasm_debug_data.scopes[class_scope->inheritance_index];
			DebugValue this_value{var.value.raw_value, zasm_debug_data.getTypeForScope(parent_class_scope)};
			Variable child(this_value, parent_class_scope->name, "this", var.color);
			child.is_synthetic = true;
			child.is_expandable = true;
			var.children.push_back(child);
		}

		auto symbols = zasm_debug_data.getChildSymbols(class_scope);
		for (auto symbol : symbols)
		{
			if (symbol->flags & SYM_FLAG_HIDDEN)
				continue;

			if (auto v = vm.readObjectMember(var.value, symbol))
			{
				DebugValue& value = v.value();
				const DebugType* type = value.type->asNonConst(zasm_debug_data);
				std::string val_str = ValueToStringSummary(value);
				Variable child(value, symbol->name, val_str, var.color);
				bool is_expandable_type = (type->tag == TYPE_ARRAY && type->extra != TYPE_CHAR32) || type->tag == TYPE_CLASS;
				child.is_expandable = is_expandable_type && !IsNull(value.raw_value, zasm_debug_data.getTypeName(value.type));
				child.symbol = symbol;
				var.children.push_back(child);
			}
		}
	}
}

expected<DebugValue, std::string> Debugger::Evaluate(std::string expression, bool suppress_engine_errors)
{
	extern refInfo *ri;

	const DebugScope* scope = selected_scope ? selected_scope : &zasm_debug_data.scopes[0];
	ExpressionEvaluator eval{zasm_debug_data, scope, vm};
	ExpressionParser parser{expression};

	auto node = parser.parseExpression();
	if (!node)
		return node.get_unexpected();

	try {
		vm.suppress_errors_in_sandbox = suppress_engine_errors;
		auto result = eval.evaluate(node.value());
		vm.suppress_errors_in_sandbox = true;
		return result;
	} catch (const std::exception& e) {
		vm.suppress_errors_in_sandbox = true;
		return make_unexpected(e.what());
	}
}

// Returns a one-line summary of a value without recursing into arrays or class members.
// Scalars print their value; arrays print "Array[n]"; class instances print the type name.
std::string Debugger::ValueToStringSummary(DebugValue value)
{
	value.type = value.type->asNonConst(zasm_debug_data);
	auto type = value.type;

	if (value.type->isVoid(zasm_debug_data))
		return "";

	if (type->isBool(zasm_debug_data))
		return value.raw_value ? "true" : "false";

	if (type->isLong(zasm_debug_data))
		return fmt::format("{}L", value.raw_value);

	if (type->isFixed(zasm_debug_data))
	{
		int64_t whole = value.raw_value / FIXED_ONE;
		int64_t frac = std::abs(value.raw_value % FIXED_ONE);
		const char* sign = (value.raw_value < 0) ? "-" : "";
		return fmt::format("{}{}.{:04}", sign, std::abs(whole), frac);
	}

	if (type->isEnum(zasm_debug_data))
	{
		auto scope = &zasm_debug_data.scopes[type->extra];
		auto symbols = zasm_debug_data.getChildSymbols(scope);
		bool isFixed = scope->type_id == TYPE_INT;
		if (type->tag == TYPE_BITFLAGS && (!isFixed || (value.raw_value % FIXED_ONE) == 0))
		{
			std::string result;
			uint32_t remaining = value.raw_value;
			if (isFixed)
				remaining /= FIXED_ONE;
			for (auto s : symbols)
			{
				uint32_t flag = s->offset;
				if (isFixed)
					flag /= FIXED_ONE;

				if (flag == 0) continue;

				bool isPowerOfTwo = (flag & (flag - 1)) == 0;
				if (!isPowerOfTwo) continue;

				if ((remaining & flag) == flag)
				{
					if (!result.empty()) 
						result += " | ";

					result += s->name;
					remaining &= ~flag;
				}
			}

			if (remaining != 0)
			{
				if (!result.empty()) 
					result += " | ";

				char hex[16];
				std::snprintf(hex, sizeof(hex), "0x%X", remaining);
				result += hex;
			}

			return fmt::format("({}) {}", scope->name, result);
		}
		else
		{
			for (auto s : symbols)
			{
				if (s->offset == value.raw_value)
					return fmt::format("({}) {}", scope->name, s->name);
			}
		}

		DebugValue value_as_number{value.raw_value, &BasicTypes[isFixed ? TYPE_INT : TYPE_LONG]};
		return fmt::format("({}) {}", scope->name, ValueToStringSummary(value_as_number));
	}

	if (type->isString(zasm_debug_data))
	{
		if (auto s = vm.readString(value.raw_value))
		{
			util::replstr(s.value(), "\n", "\\n");
			return fmt::format("\"{}\"", s.value());
		}

		return "invalid";
	}

	if (type->isArray(zasm_debug_data))
	{
		if (!value.raw_value)
			return "null (Array)";

		if (auto size = vm.readArraySize(value))
			return fmt::format("Array[{}]", size.value());

		return "invalid (Array)";
	}

	if (type->isClass(zasm_debug_data))
	{
		const DebugScope* class_scope = &zasm_debug_data.scopes[type->extra];
		if (IsNull(value.raw_value, class_scope->name)) return fmt::format("null ({})", class_scope->name);

		return class_scope->name;
	}

	return fmt::format("{}", value.raw_value);
}

// Returns a full recursive expansion of a value: scalars as in ValueToStringSummary, arrays with
// all elements listed, class instances with all member fields expanded.
// opts controls multiline formatting, array elision after 3 elements, and skipping deprecated
// members.
std::string Debugger::ValueToStringFull(DebugValue value, ValueStringOptions opts)
{
	std::set<int> seen;
	int depth = 0;
	return ValueToStringHelper(value, opts, depth, seen);
}

std::string Debugger::ValueToStringTooltip(DebugValue value)
{
	ValueStringOptions opts{.newlines = true, .elide_arrays = true, .exclude_deprecated = true, .compact_fields = false, .elide_zero_values = false};
	auto* type = value.type->asNonConst(zasm_debug_data);
	if (type->isArray(zasm_debug_data))
		opts.elide_arrays = false;
	if (type->isClass(zasm_debug_data))
	{
		const DebugScope* scope = &zasm_debug_data.scopes[type->extra];
		std::vector<const DebugSymbol*> symbols;
		while (true)
		{
			auto more = zasm_debug_data.getChildSymbols(scope);
			symbols.insert(symbols.end(), more.begin(), more.end());
			if (scope->inheritance_index == -1)
				break;
			scope = &zasm_debug_data.scopes[scope->inheritance_index];
		}
		int eligible_count = 0;
		for (auto sym : symbols)
			if (!(sym->flags & SYM_FLAG_DEPRECATED))
				eligible_count++;
		opts.elide_zero_values = eligible_count > 20;
		opts.compact_fields = eligible_count > 100;
	}

	return ValueToStringFull(value, opts);
}

std::string Debugger::ValueToStringHelper(DebugValue value, ValueStringOptions opts, int depth, std::set<int>& seen)
{
	value.type = value.type->asNonConst(zasm_debug_data);
	int raw_value = value.raw_value;

	if (value.type->isVoid(zasm_debug_data))
		return "";

	if (value.type->isBool(zasm_debug_data) || value.type->isLong(zasm_debug_data) || value.type->isFixed(zasm_debug_data) || value.type->isEnum(zasm_debug_data))
		return ValueToStringSummary(value);

	if (value.type->isString(zasm_debug_data))
		return ValueToStringSummary(value);

	if (value.type->isArray(zasm_debug_data))
	{
		if (!raw_value) return "null (Array)";

		if (seen.contains(value.raw_value))
			return "...";

		seen.insert(value.raw_value);

		auto values = vm.readArray(value);
		if (!values) return "invalid (Array)";
		if (values->empty()) return "{}";

		if (opts.elide_arrays)
			return ValueToStringSummary(value);

		std::string result = "{";

		if (opts.newlines)
		{
			// Determine if this is a "Simple" array (numbers/bools) that can be packed tightly on one
			// line.
			bool is_simple_type = false;
			const DebugType* elem_type = values->at(0).type->asNonConst(zasm_debug_data);
			is_simple_type = elem_type->isBool(zasm_debug_data) ||
							elem_type->isFixed(zasm_debug_data) ||
							elem_type->isLong(zasm_debug_data) ||
							elem_type->isUntyped(zasm_debug_data);

			// Increase indentation for the contents.
			std::string indent( (depth + 1) * 2, ' ');
			std::string closing_indent( depth * 2, ' ');

			result += "\n" + indent;
			int current_line_len = indent.length();
			const int max_line_len = 80;

			for (size_t i = 0; i < values->size(); i++)
			{
				std::string item_str = ValueToStringHelper(values->at(i), opts, depth + 1, seen);
				if (i < values->size() - 1) item_str += ", ";

				// If it's a complex type, it usually has its own newlines, so we just append.
				// If it's a simple type, we check if we need to wrap.
				if (is_simple_type)
				{
					// Check if adding this item would exceed the limit
					if (current_line_len + item_str.length() > max_line_len)
					{
						result += "\n" + indent;
						current_line_len = indent.length();
					}
					result += item_str;
					current_line_len += item_str.length();
				}
				else
				{
					result += item_str;
					if (i < values->size() - 1)
					{
						result += "\n" + indent;
						current_line_len = indent.length();
					}
				}
			}
			result += "\n" + closing_indent + "}";
		}
		else
		{
			std::vector<std::string> parts;
			for (size_t i = 0; i < values->size(); i++)
				parts.push_back(ValueToStringHelper(values->at(i), opts, depth + 1, seen));

			result += fmt::format("{}", fmt::join(parts, ", "));
			result += "}";
		}

		return result;
	}

	if (value.type->isClass(zasm_debug_data))
	{
		const DebugScope* class_scope = &zasm_debug_data.scopes[value.type->extra];

		if (IsNull(value.raw_value, class_scope->name))
		{
			int null_value = GetClassNullValue(class_scope->name);
			if (null_value != 0)
				return fmt::format("null ({})", null_value);
			return "null";
		}

		if (seen.contains(raw_value)) return "...";
		seen.insert(raw_value);

		const DebugScope* scope = class_scope;
		std::vector<const DebugScope*> scopes;
		while (scope)
		{
			scopes.push_back(scope);
			if (scope->inheritance_index == -1)
				break;

			scope = &zasm_debug_data.scopes[scope->inheritance_index];
		}
		std::reverse(scopes.begin(), scopes.end());

		std::vector<const DebugSymbol*> symbols;
		for (auto scope : scopes)
		{
			auto more_symbols = zasm_debug_data.getChildSymbols(scope);
			symbols.insert(symbols.end(), more_symbols.begin(), more_symbols.end());
		}

		std::string result = std::string(class_scope->name) + " {";

		if (opts.newlines)
		{
			std::string indent( (depth + 1) * 2, ' ');
			std::string closing_indent( depth * 2, ' ');
			const int max_col = 50;

			// Collect all field strings so we know which ones are single-line.
			struct Field { std::string name; std::string value_str; };
			std::vector<Field> fields;

			auto is_zero_or_default = [&](DebugValue v) {
				auto type = v.type->asNonConst(zasm_debug_data);
				if (type->isArray(zasm_debug_data))
				{
					if (!v.raw_value) return true;
					auto elems = vm.readArray(v);
					return elems && elems->empty();
				}
				if (type->isClass(zasm_debug_data))
				{
					const DebugScope* cs = &zasm_debug_data.scopes[type->extra];
					return IsNull(v.raw_value, cs->name);
				}
				return v.raw_value == 0;
			};

			int elided_zero_count = 0;
			for (auto symbol : symbols)
			{
				if (opts.exclude_deprecated && (symbol->flags & SYM_FLAG_DEPRECATED))
					continue;
				if (auto v = vm.readObjectMember(value, symbol))
				{
					DebugValue& member_value = v.value();
					if (opts.elide_zero_values && is_zero_or_default(member_value))
					{
						elided_zero_count++;
						continue;
					}
					fields.push_back({symbol->name, ValueToStringHelper(member_value, opts, depth + 1, seen)});
				}
			}

			int current_line_len = 0;
			for (size_t i = 0; i < fields.size(); i++)
			{
				auto& field = fields[i];
				std::string item = field.name + " = " + field.value_str;
				if (i < fields.size() - 1) item += ",";

				bool is_multiline = item.find('\n') != std::string::npos;

				if (!opts.compact_fields || is_multiline || current_line_len == 0)
				{
					result += "\n" + indent;
					current_line_len = (int)indent.size();
				}
				else if (current_line_len + 1 + (int)item.size() > max_col)
				{
					result += "\n" + indent;
					current_line_len = (int)indent.size();
				}
				else
				{
					result += " ";
					current_line_len += 1;
				}

				result += item;
				current_line_len += (int)item.size();

				if (is_multiline)
					current_line_len = 0;
			}

			if (elided_zero_count > 0)
				result += "\n" + indent + fmt::format("... {} elided vars (0, false, null, or an empty array)", elided_zero_count);
			result += "\n" + closing_indent + "}";
		}
		else
		{
			std::vector<std::string> parts;
			for (auto symbol : symbols)
			{
				if (symbol->flags & SYM_FLAG_HIDDEN)
					continue;
				if (opts.exclude_deprecated && (symbol->flags & SYM_FLAG_DEPRECATED))
					continue;

				if (auto v = vm.readObjectMember(value, symbol))
				{
					DebugValue& member_value = v.value();
					std::string val_str = ValueToStringHelper(member_value, opts, depth + 1, seen);
					parts.push_back(symbol->name + " = " + val_str);
				}
			}

			result += fmt::format("{}", fmt::join(parts, ", ")) + "}";
		}

		return result;
	}

	return fmt::format("{}", raw_value);
}

Variable Debugger::CreateVariableFromValue(const std::string& name, DebugValue value)
{
	value.type = value.type->asNonConst(zasm_debug_data);

	Variable var{};
	var.name = name;
	var.value = value;
	var.value_str = ValueToStringSummary(value); 
	var.color = GetColorForType(text_editor.GetPalette(), value.type);

	bool is_expandable_type = (value.type->tag == TYPE_ARRAY && value.type->extra != TYPE_CHAR32) || value.type->tag == TYPE_CLASS;
	var.is_expandable = is_expandable_type && !IsNull(value.raw_value, zasm_debug_data.getTypeName(value.type));

	return var;
}

void zscript_debugger_exec(pc_t pc)
{
	extern refInfo *ri;

	if (!debugger || !zasm_debug_data.exists())
		return;

	if (debugger->target_state != debugger->state)
		debugger->SetState(debugger->target_state);

	if (debugger->state == Debugger::State::PlayUntil)
	{
		if (ri->pc != debugger->pc_until)
			return;

		debugger->SetState(Debugger::State::Paused);
	}
	else if (debugger->HasBreakpoint(pc))
	{
		debugger->SetState(Debugger::State::Paused);
	}

	if (debugger->state == Debugger::State::StepOver && ri == debugger->last_ri && ri->retsp <= debugger->last_retsp)
	{
		auto [source_file, line] = zasm_debug_data.resolveLocationSourceFile(pc);
		if (source_file != debugger->last_source_file || line != debugger->last_line)
			debugger->SetState(Debugger::State::Paused);
	}

	if (debugger->state == Debugger::State::StepInto && ri == debugger->last_ri)
	{
		auto [source_file, line] = zasm_debug_data.resolveLocationSourceFile(pc);
		if (source_file != debugger->last_source_file || line != debugger->last_line)
			debugger->SetState(Debugger::State::Paused);
	}

	if (debugger->state == Debugger::State::StepOut && ri == debugger->last_ri)
	{
		if (ri->retsp < debugger->last_retsp)
			debugger->SetState(Debugger::State::Paused);
	}

	if (debugger->state == Debugger::State::Playing || debugger->state == Debugger::State::PlayUntil)
	{
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - debugger->playing_last_update_time);
		if (elapsed.count() >= 1000)
		{
			debugger->UpdateActiveScripts();
			debugger->UpdateVariables();
			debugger->playing_last_update_time = now;
		}
	}

	while (debugger && debugger->state == Debugger::State::Paused)
	{
		if (debugger->target_state != debugger->state)
			debugger->SetState(debugger->target_state);

		update_hw_screen();
	}
}

void zscript_debugger_reset()
{
	if (!debugger)
		return;

	debugger->Init();
}

Debugger* zscript_debugger_open()
{
	if (!debugger)
	{
		debugger = std::make_unique<Debugger>();
		// Only update the cfg value if the debugger wasn't opened via the CLI.
		if (!get_flag_bool("-debugger").has_value())
			zc_set_config("ZSCRIPT", "debugger", true);
	}

	return debugger.get();
}

Debugger* zscript_debugger_get_if_open()
{
	return debugger.get();
}

bool zscript_debugger_is_open()
{
	return debugger.get();
}

void zscript_debugger_clear()
{
	if (!debugger)
		return;

	debugger->Save();
	debugger->Clear();
}

void zscript_debugger_init()
{
#ifdef __EMSCRIPTEN__
	return;
#endif

	if (is_feature_enabled("-debugger", "ZSCRIPT", "debugger", false) && !jit_is_enabled())
	{
		zscript_debugger_open();
		if (auto main_display = all_get_display())
			zalleg_bring_window_to_foreground(main_display);
	}

	if (!debugger)
		return;

	debugger->Init();
}

void zscript_debugger_update()
{
	if (!debugger)
		return;

	// If the system menu is open, and the debugger state is modified, we want to close the menus.
	if (debugger->state != debugger->target_state)
		close_all_menu();

	if (!zscript_debugger_gui_update(debugger.get()))
		zscript_debugger_close();
}

void zscript_debugger_close()
{
	// Only update the cfg value if the debugger wasn't opened via the CLI.
	if (!get_flag_bool("-debugger").has_value())
		zc_set_config("ZSCRIPT", "debugger", false);
	debugger->Save();
	debugger.reset();
}
