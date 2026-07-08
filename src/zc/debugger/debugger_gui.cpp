#include "zc/debugger/debugger_gui.h"

#include "core/qst.h"
#include "base/util.h"
#include "nonstd/expected.h"
#include "components/zasm/debug_data.h"
#include "components/zasm/eval.h"
#include "zc/debugger/debugger.h"
#include "zc/scripting/types/bitmap.h"
#include "imgui.h"
#include "imgui_impl_allegro5.h"
#include "../third_party/ImGuiColorTextEdit/src/TextEditor.h"
#include "zc/render.h"
#include <climits>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <fmt/ranges.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <allegro5/allegro_windows.h>
#elif defined(__APPLE__)
// Defined in zalleg/zalleg_osx.mm.
float zalleg_osx_get_main_screen_scale_factor(void);
float zalleg_osx_get_display_scale_factor(ALLEGRO_DISPLAY* display);
void zalleg_osx_get_main_screen_usable_size(int* w, int* h);
#endif

namespace {

ALLEGRO_BITMAP* tooltip_bmp;
bool show_file_selector;

// The HiDPI scale factor for the debugger UI. On a Retina/high-DPI panel
// Allegro renders into a physical-pixel backbuffer, so the default 16px font
// and 1x widgets look tiny; we scale fonts and style by this factor to
// compensate. Pass the debugger display to query the monitor it currently
// lives on, or null to query the main screen (e.g. before the display exists).
float GetDebuggerDpiScale([[maybe_unused]] ALLEGRO_DISPLAY* display)
{
#if defined(__APPLE__)
	float scale = display ? zalleg_osx_get_display_scale_factor(display)
	                      : zalleg_osx_get_main_screen_scale_factor();
	return scale > 0.f ? scale : 1.0f;
#elif defined(_WIN32)
	// GetDpiForWindow/GetDpiForSystem only exist on Windows 10 1607+, and
	// importing them directly makes the exe fail to load on Windows 7 (which we
	// still support). Resolve them from user32 at runtime; where they don't
	// exist, don't scale.
	typedef UINT(WINAPI* GetDpiForWindowProc)(HWND);
	typedef UINT(WINAPI* GetDpiForSystemProc)(void);
	static auto get_dpi_for_window = reinterpret_cast<GetDpiForWindowProc>(
		(void*)GetProcAddress(GetModuleHandleA("user32.dll"), "GetDpiForWindow"));
	static auto get_dpi_for_system = reinterpret_cast<GetDpiForSystemProc>(
		(void*)GetProcAddress(GetModuleHandleA("user32.dll"), "GetDpiForSystem"));
	if (!get_dpi_for_window || !get_dpi_for_system)
		return 1.0f;
	UINT dpi = display ? get_dpi_for_window(al_get_win_window_handle(display))
	                   : get_dpi_for_system();
	return dpi ? dpi / 96.0f : 1.0f;
#else // X11: derived from EDID, often unreliable, so round to a whole step.
	int dpi = al_get_monitor_dpi(0);
	return dpi > 0 ? std::max(1.0f, std::roundf(dpi / 96.0f)) : 1.0f;
#endif
}

// Rescale the UI for a new DPI factor (e.g. after the window moved to a monitor
// with a different scale). FontScaleDpi makes ImGui re-rasterize fonts at the
// new size (dynamic font atlas, 1.92+); style sizes scale relative to the
// previous factor so the chosen color palette is preserved.
void ApplyDpiScale(Debugger* debugger, float new_scale)
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(new_scale / debugger->dpi_scale);
	style.FontScaleDpi = new_scale;
	debugger->dpi_scale = new_scale;
}

std::string GetCommonPathPrefix(const std::vector<SourceFile>& files)
{
	if (files.empty()) return "";

	// Start assuming the first file's path is the common prefix
	std::string prefix = files[0].path;

	for (size_t i = 1; i < files.size(); ++i)
	{
		const std::string& path = files[i].path;
		
		// Find the first character that differs
		size_t j = 0;
		while (j < prefix.size() && j < path.size() && prefix[j] == path[j])
		{
			j++;
		}
		
		// Truncate the prefix to the matching part
		prefix = prefix.substr(0, j);
	}

	// Now, ensure we only cut at directory boundaries.
	// If we have "src/data_one" and "src/data_two", the raw prefix is "src/data_".
	// We want to cut at "src/", not the middle of a filename/folder.
	size_t lastSlash = prefix.find_last_of('/');
	if (lastSlash == std::string::npos)
	{
		return ""; // No common top-level folder
	}

	// Include the trailing slash in the prefix
	return prefix.substr(0, lastSlash + 1); 
}

// Collapse folder nodes with only one folder child.
void CollapseTree(FileNode& node)
{
	for (auto& [name, child] : node.children)
	{
		if (child.isFolder)
			CollapseTree(child);
	}

	if (node.children.size() == 1)
	{
		auto it = node.children.begin();

		if (it->second.isFolder)
		{
			std::string childName = it->second.name;
			std::map<std::string, FileNode> grandChildren = std::move(it->second.children);
			node.name = node.name + "/" + childName;
			node.children = std::move(grandChildren);
			CollapseTree(node);
		}
	}
}

void BuildVirtualTree(const std::vector<SourceFile>& allFiles, FileNode& root)
{
    root.children.clear();
    
    // Calculate the offset to strip.
    std::string commonPrefix = GetCommonPathPrefix(allFiles);
    size_t offset = commonPrefix.length();

    root.name = commonPrefix.empty() ? "/" : commonPrefix; 

    for (const auto& file : allFiles)
    {
		// Ignore empty files.
		if (file.contents.empty()) continue;

        // Safety: Ensure path actually starts with the prefix (it should).
        if (file.path.size() < offset) continue;

        // Strip the prefix.
        std::string relativePath = file.path.substr(offset);

        FileNode* currentNode = &root;
        std::stringstream ss(relativePath);
        std::string segment;

        while (std::getline(ss, segment, '/'))
        {
            if (ss.eof()) // It's a file
            {
                FileNode& fileNode = currentNode->children[segment];
                fileNode.name = segment;
                fileNode.isFolder = false;
                fileNode.data = const_cast<SourceFile*>(&file);
            }
            else // It's a folder
            {
                FileNode& folderNode = currentNode->children[segment];
                folderNode.name = segment;
                folderNode.isFolder = true;
                currentNode = &folderNode;
            }
        }
    }

	// Collapse empty folders.
	for (auto& [name, child] : root.children)
	{
		if (child.isFolder)
			CollapseTree(child);
	}
}

void DrawVariableTooltip(Debugger* debugger, const Variable* var, const std::string& label)
{
	const DebugType* type = var->value.type->asNonConst(zasm_debug_data);
	std::string type_name = zasm_debug_data.getTypeName(type);
	if (type_name == "bitmap")
	{
		auto user_bitmap = checkBitmap(var->value.raw_value);
		if (user_bitmap && user_bitmap->u_bmp && !tooltip_bmp)
		{
			int w = user_bitmap->u_bmp->w;
			int h = user_bitmap->u_bmp->h;
			tooltip_bmp = al_create_bitmap(w, h);
			ALLEGRO_BITMAP* prev = al_get_target_bitmap();
			al_set_target_bitmap(tooltip_bmp);
			al_clear_to_color(al_map_rgba(0,0,0,0));
			all_render_a5_bitmap(user_bitmap->u_bmp, tooltip_bmp);
			al_set_target_bitmap(prev);
		}
	}

	ImGuiViewport* viewport = ImGui::GetMainViewport();

	// Calculate max available size (safety padding of 20px).
	float max_w = viewport->WorkSize.x - 20.0f;
	float max_h = viewport->WorkSize.y - 20.0f;

	// This forces the tooltip to never exceed the screen dimensions.
	// If the content is larger, it will simply be clipped at the bottom.
	ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(max_w, max_h));

	ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 0.0f);
	ImGui::BeginTooltip();

	if (tooltip_bmp)
		ImGui::Image((void*)tooltip_bmp, ImVec2(al_get_bitmap_width(tooltip_bmp), al_get_bitmap_height(tooltip_bmp)));

	ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, var->color);

	ImGui::TextUnformatted(label.c_str());

	ImGui::PopStyleColor();
	ImGui::PopTextWrapPos();
	ImGui::EndTooltip();
	ImGui::PopStyleVar();

	if (type->tag == TYPE_CLASS)
	{
		const DebugScope* class_scope = &zasm_debug_data.scopes[type->extra];
		while (class_scope->inheritance_index != -1)
			class_scope = &zasm_debug_data.scopes[class_scope->inheritance_index];
		if (class_scope->name == "sprite")
			debugger->highlight_sprite_id = var->value.raw_value;
	}
}

// Recursive helper to find the best fuzzy match score.
int RecursiveMatch(const char* pattern, const char* str, int currentScore, const char* matchStart)
{
	if (*pattern == '\0')
		return currentScore;

	// Scan through the string for all occurrences of the pattern.
	char pc = tolower(*pattern);
	const char* s = str;
	int bestRecursiveScore = INT_MIN;

	while (*s)
	{
		if (tolower(*s) == pc)
		{
			// Found a match for the current char. Calculate score for this step.
			int stepScore = 0;

			// Distance penalty.
			// If matchStart is null, we penalize distance from start of string slightly less.
			int distance = (matchStart == nullptr) ? (int)(s - str) : (int)(s - matchStart);
			stepScore -= distance;

			// Sequential bonus.
			if (matchStart && s == matchStart + 1)
				stepScore += 15;

			// Start of string bonus.
			if (s == str)
				stepScore += 15;

			// Separator bonus.
			// (Matches after / _ . are very important for file paths)
			if (s > str)
			{
				char prev = *(s - 1);
				if (prev == '/' || prev == '\\' || prev == '_' || prev == '.')
					stepScore += 20;
			}

			// Try to match the Rest of the pattern from s+1.
			int rest = RecursiveMatch(pattern + 1, s + 1, currentScore + stepScore, s);

			// Keep the best result.
			if (rest > bestRecursiveScore)
				bestRecursiveScore = rest;
		}

		s++;
	}

	return bestRecursiveScore;
}

// Higher score is better. Returns INT_MIN if no match.
int FuzzyMatchScore(const char* pattern, const char* str)
{
	// Fast path: Empty pattern matches everything.
	if (!*pattern) return 0;

	int score = RecursiveMatch(pattern, str, 0, nullptr);
	if (score != INT_MIN)
	{
		// Penalty: Length of the string (prefer shorter exact matches).
		score -= (int)strlen(str);
	}

	return score;
}

ImVec4 U32ToImVec4(ImU32 color)
{
	float a = ((color >> 24) & 0xFF) / 255.0f;
	float b = ((color >> 16) & 0xFF) / 255.0f;
	float g = ((color >> 8) & 0xFF) / 255.0f;
	float r = ((color) & 0xFF) / 255.0f;
	return ImVec4(r, g, b, a);
}

void PushDebuggerStyle(const TextEditor& editor)
{
	const auto& palette = editor.GetPalette();

	// Backgrounds
	ImGui::PushStyleColor(ImGuiCol_WindowBg, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Background]));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Background]));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Background]));
	
	// Text
	ImGui::PushStyleColor(ImGuiCol_Text, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Identifier]));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::LineNumber]));
	
	// Headers & Selection (Used by Selectable/CollapsingHeader)
	ImGui::PushStyleColor(ImGuiCol_Header, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Selection]));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Selection]));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Selection]));
	
	// Borders
	ImGui::PushStyleColor(ImGuiCol_Border, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::CurrentLineEdge]));
	ImGui::PushStyleColor(ImGuiCol_Separator, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::CurrentLineEdge]));

	// Tables
	ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::CurrentLineFill]));
	ImGui::PushStyleColor(ImGuiCol_TableBorderLight, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::CurrentLineEdge]));
	ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::CurrentLineEdge]));
}

void PopDebuggerStyle()
{
	ImGui::PopStyleColor(13);
}

void RenderVirtualTree(Debugger* debugger, FileNode& node)
{
	// Draw folders.
	for (auto& [name, child] : node.children)
	{
		if (!child.isFolder) continue;

		if (ImGui::TreeNode((void*)&child, "%s", child.name.c_str()))
		{
			RenderVirtualTree(debugger, child);
			ImGui::TreePop();
		}
	}

	// Draw files.
	for (auto& [name, childNode] : node.children)
	{
		if (childNode.isFolder) continue;

		bool isSelected = debugger->selected_source_file == childNode.data;
		if (ImGui::Selectable(name.c_str(), isSelected))
			debugger->SetSourceFile(childNode.data);
	}
}

void DrawMenuBar(Debugger* debugger)
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
			{
				ImGui::StyleColorsDark();
				debugger->text_editor.SetPalette(TextEditor::GetDarkPalette());
			}
			if (ImGui::MenuItem("Light palette"))
			{
				ImGui::StyleColorsLight();
				debugger->text_editor.SetPalette(TextEditor::GetLightPalette());
			}
			if (ImGui::MenuItem("Retro blue palette"))
			{
				ImGui::StyleColorsClassic();
				debugger->text_editor.SetPalette(TextEditor::GetRetroBluePalette());
			}
			ImGui::EndMenu();
		}

		if (zasm_debug_data.exists() && ImGui::BeginMenu("File"))
		{
			ImGuiIO& io = ImGui::GetIO();
			if (ImGui::MenuItem("Go to file...", io.ConfigMacOSXBehaviors ? "Cmd+P" : "Ctrl+P"))
				show_file_selector = true;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Console"))
		{
			if (ImGui::MenuItem("Copy console"))
			{
				std::string s;
				for (auto& log : debugger->console_logs)
					s += log.body + "\n";
				ImGui::SetClipboardText(s.c_str());
			}
			if (ImGui::MenuItem("Clear console"))
				debugger->console_logs.clear();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("Open docs"))
			{
				util::open_web_link("https://docs.zquestclassic.com/zscript/lang/debugger/");
			}

			ImGui::EndMenu();
		}

		if (debugger->selected_source_file)
		{
			const char* file_text = debugger->selected_source_file->path.c_str();

			// Calculate width of the text we want to draw
			float text_width = ImGui::CalcTextSize(file_text).x;

			// Move cursor to: Window Width - Text Width - Padding
			float padding_right = 20.0f;
			ImGui::SameLine(ImGui::GetWindowWidth() - text_width - padding_right);

			// Draw the text (using Disabled style to show it's just a label, not a button)
			ImGui::TextDisabled("%s", file_text);
		}

		ImGui::EndMainMenuBar();
	}
}

void DrawDebuggerControls(Debugger* debugger)
{
	ImGuiIO& io = ImGui::GetIO();
	float old_scale = ImGui::GetIO().FontGlobalScale;
	ImGui::SetWindowFontScale(1.4);

	// Calculate button size to fill the pane width.
	float pane_width = ImGui::GetContentRegionAvail().x;
	// 4 buttons, 3 spaces between them
	float spacing = ImGui::GetStyle().ItemSpacing.x;
	float btn_width = (pane_width - (spacing * 3)) / 4.0f;
	ImVec2 btn_size(btn_width, 0); // 0 height = default frame height

	bool is_running = debugger->state == Debugger::State::Playing;
	if (is_running)
	{
		// Don't call SetState directly, because we need to be within the script engine to get an
		// accurate stack frame. Defer the actual call to zscript_debugger_exec.
		if (ImGui::Button("||", btn_size) || ImGui::IsKeyPressed(ImGuiKey_F6))
			debugger->target_state = Debugger::State::Paused;
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Pause (F6)");
	}
	else
	{
		if (ImGui::Button("▶", btn_size) || ImGui::IsKeyPressed(ImGuiKey_F5))
			debugger->target_state = Debugger::State::Playing;
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Continue (F5)");
	}

	ImGui::SameLine();

	ImGui::BeginDisabled(is_running);

	if (ImGui::Button("➜", btn_size) || (!is_running && ImGui::IsKeyPressed(ImGuiKey_F10)))
		debugger->target_state = Debugger::State::StepOver;
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Step Over (F10)");

	ImGui::SameLine();

	if (ImGui::Button("↓", btn_size) || (!is_running && ImGui::IsKeyPressed(ImGuiKey_F11) && !io.KeyShift))
		debugger->target_state = Debugger::State::StepInto;
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Step Into (F11)");
	
	ImGui::SameLine();

	if (ImGui::Button("↑", btn_size) || (!is_running && ImGui::IsKeyPressed(ImGuiKey_F11) && io.KeyShift))
		debugger->target_state = Debugger::State::StepOut;
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Step Out (Shift+F11)");

	ImGui::EndDisabled();

	ImGui::SetWindowFontScale(old_scale);
}

void RenderDebugVariable(Debugger* debugger, Variable& var, std::vector<Variable*> parents, bool show_lhs = true)
{
	auto RevealDeclaration = [&]()
	{
		auto [source_file, line] = zasm_debug_data.getSymbolLocation(var.symbol);
		if (source_file && line)
			debugger->SetSourceFileAndLine(source_file, line);
	};

	auto ShowContextMenu = [&]()
	{
		if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Copy name"))
				ImGui::SetClipboardText(var.name.c_str());

			if (ImGui::MenuItem("Copy value"))
				ImGui::SetClipboardText(debugger->ValueToStringFull(DebugValue{var.value.raw_value, var.value.type}, {.newlines = true}).c_str());

			if (var.symbol && ImGui::MenuItem("Reveal declaration"))
				RevealDeclaration();

			std::string expression;
			for (auto& parent : parents)
			{
				if (parent->is_synthetic)
					continue;

				expression += parent->name;
				if (!parent->value.type->isArray(zasm_debug_data))
					expression += "->";
			}
			expression += var.name;

			auto it = std::find_if(debugger->watches.begin(), debugger->watches.end(), [&](auto& w){ return w.expression == expression; });
			bool watch_exists = it != debugger->watches.end();
			if (ImGui::MenuItem(watch_exists ? "Unwatch (expression)" : "Watch (expression)"))
			{
				if (watch_exists)
					debugger->watches.erase(it);
				else
					debugger->AddWatchExpression(std::move(expression));
			}

			bool can_watch_value = var.value.type->isClass(zasm_debug_data) || var.value.type->isArray(zasm_debug_data);
			if (can_watch_value)
			{
				auto it = std::find_if(debugger->watches.begin(), debugger->watches.end(), [&](auto& w){ return w.value == var.value; });
				bool watch_exists = it != debugger->watches.end();
				if (ImGui::MenuItem(watch_exists ? "Unwatch (value)" : "Watch (value)"))
				{
					if (watch_exists)
						debugger->watches.erase(it);
					else
						debugger->AddWatchValue(var.value, fmt::format("({})", expression));
				}
			}

			ImGui::EndPopup();
		}
	};

	auto HandleOnClick = [&]()
	{
		if (var.symbol && ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			RevealDeclaration();
	};

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_OpenOnArrow;
	bool row_hovered = false;

	int id = 0;
	auto key = std::make_pair(var.name, parents.size());
	if (auto it = debugger->variable_name_to_imgui_id.find(key); it != debugger->variable_name_to_imgui_id.end())
		id = it->second;
	else
		id = debugger->variable_name_to_imgui_id[key] = debugger->next_imgui_id++;

	ImGui::PushID(id);

	if (!var.is_expandable)
	{
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		if (show_lhs)
		{
			ImGui::TreeNodeEx(var.name.c_str(), flags, "%s", var.name.c_str());
			ShowContextMenu();
			HandleOnClick();

			if (ImGui::IsItemHovered()) row_hovered = true;

			ImGui::SameLine();
			ImGui::Text("=");
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Text, var.color);
			ImGui::TextUnformatted(var.value_str.c_str());
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, var.color);
			ImGui::TreeNodeEx(var.name.c_str(), flags, "%s", var.value_str.c_str());
			ImGui::PopStyleColor();
			ShowContextMenu();
			HandleOnClick();
		}

		if (ImGui::IsItemHovered()) row_hovered = true;
	}
	else
	{
		bool open = ImGui::TreeNodeEx(var.name.c_str(), flags, "%s", show_lhs ? var.name.c_str() : "");
		ShowContextMenu();
		HandleOnClick();

		if (ImGui::IsItemHovered()) row_hovered = true;

		// Draw the summary on the same line (e.g. "myArray = Array[5]").
		ImGui::SameLine();
		if (show_lhs)
		{
			ImGui::Text("=");
			ImGui::SameLine();
		}

		ImGui::PushStyleColor(ImGuiCol_Text, var.color);
		ImGui::TextUnformatted(var.value_str.c_str());
		ImGui::PopStyleColor();

		if (ImGui::IsItemHovered()) row_hovered = true;

		if (open)
		{
			if (!var.children_populated)
				debugger->FetchChildren(var);

			parents.push_back(&var);
			for (auto& child : var.children)
				RenderDebugVariable(debugger, child, parents);
			parents.pop_back();

			ImGui::TreePop();
		}
	}

	ImGui::PopID();

	if (row_hovered)
		DrawVariableTooltip(debugger, &var, fmt::format("{} = {}", var.name, var.value_str));
}

void DrawLeftPaneContent(Debugger* debugger)
{
	const auto& palette = debugger->text_editor.GetPalette();
	auto stack_trace = debugger->current_stack_trace;

	static bool show_files = false;
	static bool show_variables = true;
	static bool show_watch = false;
	static bool show_breaks = true;
	static bool show_scripts = true;
	static bool show_stack = true;

	// Calculate a flex-like layout.
	const float weight_files = 2.0f;
	const float weight_variables = 3.0f;
	const float weight_watch = 1.0f;
	const float weight_breaks = 1.0f;
	const float weight_scripts = 1.0f;
	const float weight_stack = 1.0f;

	float total_weight = 0.0f;
	int open_count = 0;

	if (show_files)
	{
		total_weight += weight_files;
		open_count++;
	}
	if (show_variables)
	{
		total_weight += weight_variables;
		open_count++;
	}
	if (show_watch)
	{
		total_weight += weight_watch;
		open_count++;
	}
	if (show_breaks)
	{
		total_weight += weight_breaks;
		open_count++;
	}
	if (show_scripts)
	{
		total_weight += weight_scripts;
		open_count++;
	}
	if (show_stack)
	{
		total_weight += weight_stack;
		open_count++;
	}

	float header_height = ImGui::GetFrameHeight();
	float spacing = ImGui::GetStyle().ItemSpacing.y;
	float footer_height = ImGui::GetFrameHeight() + spacing * 2.0f;
	int total_sections = 6;
	int total_items = total_sections + open_count; 
	int total_gaps = (total_items > 0) ? (total_items - 1) : 0;
	
	float available_h = ImGui::GetContentRegionAvail().y - (total_sections * header_height) - (total_gaps * spacing) - footer_height;
	if (available_h < 0.0f) available_h = 0.0f;

	auto GetHeightFor = [&](float weight) -> float {
		if (total_weight <= 0.0f) return 0.0f;
		return floorf(available_h * (weight / total_weight));
	};

	if (ImGui::CollapsingHeader("Files", nullptr))
	{
		show_files = true; 
		ImGui::BeginChild("FilesContent", ImVec2(0, GetHeightFor(weight_files)), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 6.0f);
		RenderVirtualTree(debugger, debugger->root_node);
		ImGui::PopStyleVar();
		ImGui::EndChild();
	}
	else
	{
		show_files = false;
	}

	if (ImGui::CollapsingHeader("Variables", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
	{
		show_variables = true; 
		ImGui::BeginChild("VariablesContent", ImVec2(0, GetHeightFor(weight_variables)), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 6.0f);

		if (debugger->state == Debugger::State::Paused)
		{
			if (debugger->variables_dirty)
			{
				debugger->UpdateVariables();
				debugger->variables_dirty = false;
			}

			for (auto& [name, variables] : debugger->variable_groups)
			{
				if (variables.empty())
					continue;

				ImGui::Text("%s", name.c_str());

				for (auto& var : variables)
				{
					std::vector<Variable*> parents;
					RenderDebugVariable(debugger, var, parents);
				}
			}
		}
		else
		{
			ImGui::TextDisabled("Not paused.");
		}

		ImGui::PopStyleVar();
		ImGui::EndChild();
	}
	else
	{
		show_variables = false;
	}

	if (ImGui::CollapsingHeader("Watch", nullptr, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap))
	{
		show_watch = true; 

		// Buttons.
		{
			int num_buttons = 2;
			float button_w = 22.0f;
			float spacing = ImGui::GetStyle().ItemSpacing.x;
			float buttons_total_w = (button_w * num_buttons) + (spacing * num_buttons);

			ImGui::SameLine(ImGui::GetWindowWidth() - buttons_total_w);

			if (ImGui::Button("+##watch", ImVec2(button_w, 0)))
				ImGui::OpenPopup("AddWatchPopup");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Add Expression");

			if (ImGui::BeginPopup("AddWatchPopup"))
			{
				static char buf[256] = "";
				
				// Auto-focus the input field when popup opens.
				if (ImGui::IsWindowAppearing())
					ImGui::SetKeyboardFocusHere(0);

				// Can submit with "Enter".
				if (ImGui::InputText("##expr", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					std::string expr = buf;
					if (!expr.empty())
					{
						debugger->AddWatchExpression(expr);
						buf[0] = 0;
						ImGui::CloseCurrentPopup();
					}
				}

				// Or can submit w/ mouse click.
				ImGui::SameLine();
				if (ImGui::Button("Add"))
				{
					std::string expr = buf;
					if (!expr.empty())
					{
						debugger->AddWatchExpression(expr);
						buf[0] = 0;
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::EndPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("-##watch", ImVec2(button_w, 0)))
				debugger->RemoveWatches();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Clear All");
		}

		ImGui::BeginChild("WatchContent", ImVec2(0, GetHeightFor(weight_watch)), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 6.0f);

		for (auto& var : debugger->watch_variables)
		{
			std::vector<Variable*> parents;
			RenderDebugVariable(debugger, var, parents);
		}

		ImGui::PopStyleVar();
		ImGui::EndChild();
	}
	else
	{
		show_watch = false;
	}

	if (ImGui::CollapsingHeader("Breakpoints", nullptr, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap))
	{
		show_breaks = true;

		// Show only one visual breakpoint per line of code, even if it maps to multiple pcs (such
		// as for templated functions).
		if (debugger->breakpoints_dirty)
		{
			std::set<std::pair<const SourceFile*, int>> seen;
			debugger->breakpoints_deduped.clear();
			for (auto& breakpoint : debugger->breakpoints)
			{
				auto pair = std::make_pair(breakpoint.source_file, breakpoint.line);
				if (seen.contains(pair))
					continue;

				seen.insert(pair);
				debugger->breakpoints_deduped.push_back(breakpoint);
			}

			debugger->breakpoints_deduped.push_back(Breakpoint{
				.type = Breakpoint::Type::ScriptStart,
				.enabled = debugger->break_on_new_script,
			});
			debugger->breakpoints_deduped.push_back(Breakpoint{
				.type = Breakpoint::Type::Error,
				.enabled = debugger->break_on_error,
			});
			debugger->breakpoints_dirty = false;
		}

		// Buttons.
		{
			int num_buttons = 3;
			float button_w = 22.0f;
			float spacing = ImGui::GetStyle().ItemSpacing.x;
			float buttons_total_w = (button_w * num_buttons) + (spacing * num_buttons);

			ImGui::SameLine(ImGui::GetWindowWidth() - buttons_total_w);

			if (ImGui::Button("√##break", ImVec2(button_w, 0)))
			{
				for (auto& breakpoint : debugger->breakpoints) breakpoint.enabled = true;
				debugger->breakpoints_dirty = true;
				debugger->UpdateTextEditorBreakpoints();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable All");

			ImGui::SameLine();

			if (ImGui::Button("X##break", ImVec2(button_w, 0)))
			{
				for (auto& breakpoint : debugger->breakpoints) breakpoint.enabled = false;
				debugger->breakpoints_dirty = true;
				debugger->UpdateTextEditorBreakpoints();
			}
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Disable All");

			ImGui::SameLine();

			if (ImGui::Button("-##break", ImVec2(button_w, 0)))
				debugger->RemoveBreakpoints();
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Clear All");
		}

		ImGui::BeginChild("BreaksContent", ImVec2(0, GetHeightFor(weight_breaks)), true);

		auto& breaks = debugger->breakpoints_deduped;
		if (breaks.empty())
		{
			ImGui::TextDisabled("No breakpoints set.");
		}
		else
		{
			for (int i = 0; i < breaks.size(); ++i)
			{
				auto& breakpoint = breaks[i];

				// Checkbox for enabled.
				ImGui::PushID(i); 
				bool enabled = breakpoint.enabled;

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

				if (ImGui::Checkbox("##enabled", &enabled))
				{
					breakpoint.enabled = enabled;
					if (breakpoint.type == Breakpoint::Type::Normal)
					{
						for (auto& master_bp : debugger->breakpoints)
						{
							if (master_bp.source_file == breakpoint.source_file && master_bp.line == breakpoint.line)
								master_bp.enabled = enabled;
						}
						debugger->UpdateTextEditorBreakpoints();
					}
					else if (breakpoint.type == Breakpoint::Type::ScriptStart)
					{
						debugger->break_on_new_script = enabled;
					}
					else if (breakpoint.type == Breakpoint::Type::Error)
					{
						debugger->break_on_error = enabled;
					}
				}

				ImGui::PopStyleVar();
				ImGui::PopID();

				ImGui::SameLine();

				std::string label;
				if (breakpoint.type == Breakpoint::Type::Normal)
				{
					// Just show filename and line.
					const std::string& path = breakpoint.source_file->path;
					size_t pos = path.find_last_of("/\\") + 1;
					label = fmt::format("{}:{}", path.substr(pos), breakpoint.line);
				}
				else if (breakpoint.type == Breakpoint::Type::ScriptStart)
				{
					label = "on script start";
				}
				else if (breakpoint.type == Breakpoint::Type::Error)
				{
					label = "on error";
				}

				if (!enabled)
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

				if (ImGui::Selectable(label.c_str()))
				{
					if (breakpoint.source_file)
					{
						debugger->SetSourceFileAndLine(breakpoint.source_file, breakpoint.line);
						ImGui::SetWindowFocus("EditorPane");
					}
				}

				if (!enabled)
					ImGui::PopStyleColor();

				if (breakpoint.source_file && ImGui::IsItemHovered())
				{
					const std::string& path = breakpoint.source_file->path;
					std::string tooltip = fmt::format("{}:{}", path, breakpoint.line);
					ImGui::SetTooltip("%s", tooltip.c_str());
				}
			}
		}
		ImGui::EndChild();
	}
	else
	{
		show_breaks = false;
	}

	int frame_index = debugger->selected_stack_frame_index;
	if (debugger->state == Debugger::State::Paused && stack_trace && debugger->selected_source_file == stack_trace->frames[frame_index].source_file)
		debugger->text_editor.SetLineHighlights({stack_trace->frames[frame_index].line});
	else
		debugger->text_editor.SetLineHighlights({});

	if (ImGui::CollapsingHeader("Scripts", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
	{
		show_scripts = true;
		ImGui::BeginChild("ScriptsContent", ImVec2(0, GetHeightFor(weight_scripts)), true);

		for (int i = 0; i < debugger->active_scripts.size(); ++i)
		{
			auto& active_script = debugger->active_scripts[i];
			ImGui::PushID(i);

			bool is_selected = debugger->selected_script == &active_script;
			if (is_selected)
				ImGui::PushStyleColor(ImGuiCol_Text, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Keyword]));

			ImGui::BeginDisabled(debugger->state != Debugger::State::Paused);

			if (ImGui::Selectable(active_script.name.c_str()))
			{
				debugger->SetSelectedScriptIndex(i);
			}

			if (active_script.sprite_id && ImGui::IsItemHovered())
				debugger->highlight_sprite_id = active_script.sprite_id;

			ImGui::EndDisabled();

			if (is_selected) ImGui::PopStyleColor();
			ImGui::PopID();
		}
		ImGui::EndChild();
	}
	else
	{
		show_scripts = false;
	}

	if (ImGui::CollapsingHeader("Call Stack", nullptr, ImGuiTreeNodeFlags_DefaultOpen))
	{
		show_stack = true;
		ImGui::BeginChild("StackContent", ImVec2(0, GetHeightFor(weight_stack)), true);

		if (!stack_trace)
		{
			ImGui::TextDisabled("Not paused.");
		}
		else
		{
			for (int i = 0; i < stack_trace->frames.size(); ++i)
			{
				const auto& frame = stack_trace->frames[i];
				std::string label = frame.function_name.empty() ? frame.to_short_string() : frame.function_name;

				bool is_selected = i == debugger->selected_stack_frame_index;
				if (is_selected)
					ImGui::PushStyleColor(ImGuiCol_Text, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Keyword]));

				if (ImGui::Selectable(label.c_str()))
				{
					if (frame.source_file)
						debugger->SetSourceFileAndLine(frame.source_file, frame.line);

					debugger->SetSelectedStackFrameIndex(i);
				}

				if (is_selected) ImGui::PopStyleColor();

				if (frame.source_file && ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", frame.to_string().c_str());
			}
		}
		ImGui::EndChild();
	}
	else
	{
		show_stack = false;
	}

	DrawDebuggerControls(debugger);
}

int ConsoleInputCallback(ImGuiInputTextCallbackData* data)
{
	Debugger* debugger = (Debugger*)data->UserData;
	
	if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
	{
		const int prev_history_pos = debugger->history_pos;
		
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (debugger->history_pos == -1)
				debugger->history_pos = debugger->console_command_history.size() - 1;
			else if (debugger->history_pos > 0)
				debugger->history_pos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (debugger->history_pos != -1)
			{
				if (++debugger->history_pos >= debugger->console_command_history.size())
					debugger->history_pos = -1;
			}
		}

		// If the position changed, update the input buffer.
		if (prev_history_pos != debugger->history_pos)
		{
			const char* history_str = (debugger->history_pos >= 0) ? debugger->console_command_history[debugger->history_pos].c_str() : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
		}
	}

	return 0;
}

void DrawConsole(Debugger* debugger)
{
	const auto& palette = debugger->text_editor.GetPalette();
	bool draw_input = zasm_debug_data.exists();
	float input_height = draw_input ? ImGui::GetFrameHeightWithSpacing() : 0;

	if (ImGui::BeginChild("ConsoleLog", ImVec2(0, -input_height), false, 0))
	{
		if (ImGui::BeginTable("ConsoleTable", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
		{
			ImGui::TableSetupColumn("Output", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthFixed, 140.0f);

			auto RenderLogNode = [&](int i)
			{
				const auto& log = debugger->console_logs[i];
				const StackFrame* top_frame = log.stack_trace && log.stack_trace->frames.size() ? &log.stack_trace->frames.front() : nullptr;
				bool has_stack = log.stack_trace && !log.stack_trace->frames.empty();

				ImGui::PushID(i);
				ImGui::TableNextRow();

				ImGui::TableNextColumn();

				if (log.variable)
				{
					// Draw a small marker to show the expression that ran.
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 1, 0, 1));
					ImGui::Text("=>");
					ImGui::PopStyleColor();
					ImGui::SameLine();

					std::vector<Variable*> parents;
					bool show_lhs = false;
					RenderDebugVariable(debugger, *log.variable, parents, show_lhs);
				}
				else
				{
					bool is_open = false;

					// Arrow Logic
					if (has_stack)
					{
						ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
						// Use invisible label ("##" + ID) so we just get the arrow
						ImGui::PushStyleColor(ImGuiCol_Text, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Comment]));
						is_open = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", "");

						if (ImGui::IsItemClicked() && i == debugger->console_logs.size() - 1)
							debugger->console_scroll_to_bottom = true;

						ImGui::PopStyleColor();
						ImGui::SameLine();
					}
					else
					{
						// Spacer for alignment if no stack trace
						ImGui::Dummy(ImVec2(ImGui::GetTreeNodeToLabelSpacing() - ImGui::GetStyle().ItemSpacing.x, 0));
						ImGui::SameLine();
					}

					// Message Body
					ImGui::TextWrapped("%s", log.body.c_str());

					// Context Menu
					if (ImGui::BeginPopupContextItem("context_menu"))
					{
						if (ImGui::MenuItem("Copy"))
						{
							if (log.stack_trace)
								ImGui::SetClipboardText(fmt::format("{}{}\n", log.body, log.stack_trace->to_string()).c_str());
							else
								ImGui::SetClipboardText(log.body.c_str());
						}
						ImGui::EndPopup();
					}

					// Clickable Message Body (Navigate to source)
					if (top_frame && top_frame->source_file && ImGui::IsItemClicked())
						debugger->SetSourceFileAndLine(top_frame->source_file, top_frame->line);

					// Stack Trace Rendering (renders below the message in the same column)
					if (is_open)
					{
						ImGui::Indent(ImGui::GetTreeNodeToLabelSpacing() / 2);
						ImGui::PushStyleColor(ImGuiCol_Text, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Comment]));

						for (int j = 0; j < log.stack_trace->frames.size(); ++j)
						{
							const auto& frame = log.stack_trace->frames[j];
							ImGui::PushID(j);

							if (ImGui::Selectable(frame.to_string().c_str()))
							{
								if (frame.source_file)
									debugger->SetSourceFileAndLine(frame.source_file, frame.line);
							}

							ImGui::PopID();
						}

						ImGui::PopStyleColor();
						ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing() / 2);
						ImGui::TreePop();
					}
				}

				ImGui::TableNextColumn();

				ImGui::PushStyleColor(ImGuiCol_Text, U32ToImVec4(palette[(int)TextEditor::PaletteIndex::Preprocessor]));

				if (top_frame && top_frame->source_file)
				{
					if (ImGui::Selectable(top_frame->to_short_string().c_str(), false))
						debugger->SetSourceFileAndLine(top_frame->source_file, top_frame->line);
				}

				ImGui::PopStyleColor();

				ImGui::PopID();
			};

			int list_size = debugger->console_logs.size();

			// Exclude the last 5 items from the clipper entirely
			int clipped_size = std::max(0, list_size - 5);

			ImGuiListClipper clipper;
			clipper.Begin(clipped_size);

			while (clipper.Step())
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
					RenderLogNode(i);

			// Always render the last 5 items manually so their true height is never culled.
			int manual_start = std::max(0, list_size - 5);
			for (int i = manual_start; i < list_size; ++i)
				RenderLogNode(i);

			ImGui::EndTable();
		}

		if (debugger->console_scroll_to_bottom)
		{
			ImGui::SetScrollHereY(1.0f);
			debugger->console_scroll_to_bottom = false;
		}
	}
	ImGui::EndChild();

	if (!draw_input)
		return;

	static char input_buf[1024*5] = "";

	// Stretch the input box to the full width of the window.
	ImGui::PushItemWidth(-FLT_MIN);
	ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory;

	if (ImGui::InputTextWithHint("##ConsoleInput", "Evaluate expression...", input_buf, IM_ARRAYSIZE(input_buf), flags, ConsoleInputCallback, (void*)debugger))
	{
		std::string cmd = input_buf;

		if (!cmd.empty())
		{
			debugger->AddToConsoleExpressionHistory(cmd);
			debugger->AddConsoleMessage("> " + cmd);

			if (auto v = debugger->Evaluate(cmd, false))
				debugger->AddConsoleDebugValue(cmd, v.value());
			else
				debugger->AddConsoleMessage(v.error());

			input_buf[0] = '\0';
			debugger->history_pos = -1;
			ImGui::SetKeyboardFocusHere(-1);
			debugger->console_scroll_to_bottom = true;
			debugger->UpdateVariables();
		}
	}
	ImGui::PopItemWidth();
}

void DrawFileSelector(Debugger* debugger)
{
	ImGuiIO& io = ImGui::GetIO();

	bool shortcut_press = io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_P, false);
	if (shortcut_press || show_file_selector)
	{
		ImGui::OpenPopup("FileSelector");
		show_file_selector = false;
	}

	// Position the modal at the top of the window, and in the center horizontally.
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImVec2 pos(viewport->GetCenter().x, viewport->Pos.y);
	ImGui::SetNextWindowPos(pos, ImGuiCond_Appearing, ImVec2(0.5f, 0.0f));
	ImGui::SetNextWindowSize(ImVec2(600 * debugger->dpi_scale, 400 * debugger->dpi_scale));

	bool open = true;
	if (ImGui::BeginPopupModal("FileSelector", &open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize))
	{
		// Check for click outside modal.
		if (ImGui::IsMouseClicked(0) && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
			ImGui::CloseCurrentPopup();

		static char search_buf[1024] = "";
		static std::vector<const SourceFile*> filtered_files;
		static std::vector<std::pair<int, const SourceFile*>> ranked_files;
		static int selected_index = 0;
		static bool scroll_to_selection = false;

		if (ImGui::IsWindowAppearing())
		{
			search_buf[0] = 0;
			selected_index = 0;
			filtered_files.clear();
			for (const auto& source_file : zasm_debug_data.source_files)
				filtered_files.push_back(&source_file);

			scroll_to_selection = true;
		}

		if (!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
			ImGui::SetKeyboardFocusHere(0);

		ImGui::PushItemWidth(-FLT_MIN); 
		bool enter_pressed = ImGui::InputText("##Search", search_buf, IM_ARRAYSIZE(search_buf), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

		if (ImGui::IsItemEdited())
		{
			ranked_files.clear();
			for (const auto& source_file : zasm_debug_data.source_files)
			{
				int score = FuzzyMatchScore(search_buf, source_file.path.c_str());
				if (score != INT_MIN)
					ranked_files.push_back({ score, &source_file });
			}
			std::sort(ranked_files.begin(), ranked_files.end(), [](const auto& a, const auto& b) {
				return a.first > b.first;
			});

			filtered_files.clear();
			for (const auto& pair : ranked_files)
				filtered_files.push_back(pair.second);

			selected_index = 0;
			scroll_to_selection = true;
		}
		ImGui::PopItemWidth();

		// Navigation.
		if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))
		{
			selected_index++;
			if (selected_index >= (int)filtered_files.size()) selected_index = 0;
			scroll_to_selection = true;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))
		{
			selected_index--;
			if (selected_index < 0) selected_index = filtered_files.empty() ? 0 : (int)filtered_files.size() - 1;
			scroll_to_selection = true;
		}
		else if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			ImGui::CloseCurrentPopup();
		}

		if (enter_pressed || ImGui::IsKeyPressed(ImGuiKey_Enter))
		{
			if (!filtered_files.empty())
			{
				debugger->SetSourceFile(const_cast<SourceFile*>(filtered_files[selected_index]));
				ImGui::CloseCurrentPopup();
			}
		}

		// List results.
		if (ImGui::BeginChild("Results", ImVec2(0, 0), true, ImGuiWindowFlags_NoNav))
		{
			for (int i = 0; i < filtered_files.size(); ++i)
			{
				const SourceFile* file = filtered_files[i];
				bool is_highlighted = (i == selected_index);

				if (ImGui::Selectable(file->path.c_str(), is_highlighted))
				{
					debugger->SetSourceFile(const_cast<SourceFile*>(file));
					ImGui::CloseCurrentPopup();
				}

				if (is_highlighted && scroll_to_selection)
				{
					ImGui::SetScrollHereY(0.5f);
					scroll_to_selection = false;
				}
			}
		}
		ImGui::EndChild();

		ImGui::EndPopup();
	}
}

void DrawDebuggerWindowContents(Debugger* debugger)
{
	if (!zasm_debug_data.exists())
	{
		DrawConsole(debugger);
		return;
	}

	// These pixel sizes are stored in physical pixels (matching the backbuffer
	// and ImGui's coordinate space), so the defaults and minimums scale with DPI.
	float scale = debugger->dpi_scale;
	static float console_height = 300.0f * scale;
	static float left_pane_width = 300.0f * scale;

	const float min_console_height = 100.0f * scale;
	const float min_left_pane_width = 150.0f * scale;
	const float splitter_thickness = 10.0f * scale;

	float avail_width = ImGui::GetContentRegionAvail().x;
	float avail_height = ImGui::GetContentRegionAvail().y;

	if (console_height < min_console_height) console_height = min_console_height;
	if (console_height > avail_height - min_console_height) console_height = avail_height - min_console_height;
	if (left_pane_width < min_left_pane_width) left_pane_width = min_left_pane_width;
	if (left_pane_width > avail_width - min_left_pane_width) left_pane_width = avail_width - min_left_pane_width;

	float top_section_height = avail_height - console_height - splitter_thickness;

	// Top section (left pane & editor).
	{
		// Left pane (files, variables, breakpoints, call stack).
		ImGui::BeginChild("LeftPane", ImVec2(left_pane_width, top_section_height), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_HorizontalScrollbar);
		DrawLeftPaneContent(debugger);
		ImGui::EndChild();

		ImGui::SameLine(0.0f, 0.0f);

		// Vertical splitter (left pane <-> editor).
		// Draw an invisible button exactly where the cursor is (between the panes).
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

		ImGui::Button("##vsplitter", ImVec2(splitter_thickness, top_section_height));

		ImGui::PopStyleColor(3);

		if (ImGui::IsItemActive())
			left_pane_width += ImGui::GetIO().MouseDelta.x;
		
		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

		ImGui::SameLine(0.0f, 0.0f);

		// Right pane (text editor).
		ImGui::BeginChild("EditorPane", ImVec2(0, top_section_height), false);
		debugger->text_editor.Render("ZQuest Classic Debugger");
		ImGui::EndChild();
	}

	// Top / bottom splitter.
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY()); 

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));

		ImGui::Button("##hsplitter", ImVec2(-1, splitter_thickness));

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		if (ImGui::IsItemActive()) 
			console_height -= ImGui::GetIO().MouseDelta.y;

		if (ImGui::IsItemHovered() || ImGui::IsItemActive())
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

		ImGui::Separator(); 
	}

	// Bottom (console).
	{
		ImGui::BeginChild("ConsolePane", ImVec2(0, 0), true);
		DrawConsole(debugger);
		ImGui::EndChild();
	}
}

} // end namespace

void Debugger::InitGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();

	dpi_scale = GetDebuggerDpiScale(nullptr);

	// Scale fonts and style for HiDPI. FontScaleDpi makes ImGui rasterize the
	// font at the scaled size so text stays crisp (dynamic font atlas, 1.92+),
	// and ScaleAllSizes scales padding, spacing, scrollbars, etc.
	if (!io.Fonts->AddFontFromFileTTF("ProggyVector-Regular.ttf", 16.0f))
		io.Fonts->AddFontDefault();
	ImGui::GetStyle().ScaleAllSizes(dpi_scale);
	ImGui::GetStyle().FontScaleDpi = dpi_scale;

	// The desired logical size, expressed in the physical pixels that Allegro
	// expects, so the window opens at a sensible on-screen size.
	int w = 1200 * dpi_scale;
	int h = 1100 * dpi_scale;
#if defined(__APPLE__)
	// Clamp to the usable area (minus menu bar/dock), reserving room for the
	// window title bar, so the window isn't pushed partly off the top of the
	// screen. al_get_monitor_info reports the full frame including the menu bar,
	// which is too generous here.
	int usable_w = 0, usable_h = 0;
	zalleg_osx_get_main_screen_usable_size(&usable_w, &usable_h);
	if (usable_w > 0) w = std::min(w, usable_w);
	if (usable_h > 0) h = std::min(h, usable_h - (int)(40 * dpi_scale));
#else
	ALLEGRO_MONITOR_INFO info; // Monitor bounds are reported in physical pixels.
	if (al_get_monitor_info(0, &info))
	{
		w = std::min(w, info.x2 - info.x1);
		h = std::min(h, info.y2 - info.y1 - (int)(40 * dpi_scale));
	}
#endif

	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	al_set_new_display_option(ALLEGRO_AUTO_CONVERT_BITMAPS, 1, ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_VSYNC, 2, ALLEGRO_REQUIRE);
	display = al_create_display(w, h);

	al_set_window_title(display, "Debugger");
	ImGui_ImplAllegro5_Init(display);

	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());

	text_editor.SetReadOnly(true);
	text_editor.SetShowWhitespaces(false);
	text_editor.SetBreakpointCallback([&](int line, bool added, bool enabled){
		auto pcs = zasm_debug_data.resolveAllPcsFromSourceLocation(selected_source_file, line);
		for (pc_t pc : pcs)
		{
			if (added)
				AddBreakpoint(selected_source_file, line, pc, enabled);
			else
				RemoveBreakpoint(pc);
		}
	});
	text_editor.SetHoverCallback([&](const std::string& word) {
		if (state != Debugger::State::Paused) return;

		static DebugValue prev_value;
		static std::string prev_str;
		const Variable* var = FindVariable(word);
		if (!var)
		{
			auto expr = Evaluate(word, true);
			if (!expr) return;

			DebugValue value = expr.value();
			if (value.type->isVoid(zasm_debug_data)) return;

			if (value != prev_value)
			{
				prev_value = value;
				prev_str = ValueToStringTooltip(value);
			}

			auto var = CreateVariableFromValue("", value);
			DrawVariableTooltip(this, &var, prev_str);
			return;
		}

		if (var->value != prev_value)
		{
			prev_value = var->value;
			prev_str = ValueToStringTooltip(var->value);
		}

		DrawVariableTooltip(this, var, prev_str);
	});
}

bool zscript_debugger_gui_update(Debugger* debugger)
{
	al_set_target_backbuffer(debugger->display);

	if (debugger->root_node.children.empty())
		BuildVirtualTree(zasm_debug_data.source_files, debugger->root_node);

	bool running = true;

	ALLEGRO_EVENT ev;
	while (al_get_next_event(debugger->queue, &ev))
	{
		// Always handle some key down/up events, even if the main display is focused.
		switch (ev.type)
		{
			case ALLEGRO_EVENT_KEY_DOWN:
			case ALLEGRO_EVENT_KEY_UP:
			{
				switch (ev.keyboard.keycode)
				{
					case ALLEGRO_KEY_F5:
					case ALLEGRO_KEY_F6:
					case ALLEGRO_KEY_F10:
					case ALLEGRO_KEY_F11:
					{
						ev.keyboard.display = debugger->display;
					}
				}
			}
		}
		ImGui_ImplAllegro5_ProcessEvent(&ev);
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) running = false;
		if (ev.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
		{
			ImGui_ImplAllegro5_InvalidateDeviceObjects();
			al_acknowledge_resize(debugger->display);

			// The window may have moved to a monitor with a different DPI.
			float new_scale = GetDebuggerDpiScale(debugger->display);
			if (new_scale > 0.f && new_scale != debugger->dpi_scale)
				ApplyDpiScale(debugger, new_scale);

			ImGui_ImplAllegro5_CreateDeviceObjects();
		}
	}

	ImGui_ImplAllegro5_NewFrame();
	ImGui::NewFrame();

	DrawMenuBar(debugger);

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	ImGui::Begin("Window", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);
	PushDebuggerStyle(debugger->text_editor);
	DrawDebuggerWindowContents(debugger);
	PopDebuggerStyle();
	ImGui::End(); // End Main Window

	DrawFileSelector(debugger);

	// Render.
	ImGui::Render();
	ImGui_ImplAllegro5_RenderDrawData(ImGui::GetDrawData());

	if (tooltip_bmp)
	{
		al_destroy_bitmap(tooltip_bmp);
		tooltip_bmp = nullptr;
	}

	al_flip_display();
	al_set_target_backbuffer(all_get_display());

	if (debugger->state == Debugger::State::Paused && info_bmp_enabled())
	{
		// The info layer holds the walkability/hitbox cheat overlays drawn during the
		// last game frame. The game isn't redrawing while paused, so snapshot those
		// overlays on the first paused frame and restore them each frame afterwards.
		// This lets us composite the transient sprite highlight on top without
		// permanently erasing the cheat overlays (which made them vanish when paused).
		if (!debugger->info_bmp_saved)
		{
			save_info_bmp();
			debugger->info_bmp_saved = true;
		}
		else
		{
			restore_info_bmp();
		}

		if (debugger->highlight_sprite_id)
		{
			if (sprite* spr = sprite::getByUID(debugger->highlight_sprite_id))
			{
				extern viewport_t viewport;

				start_info_bmp();
				int x = -viewport.x;
				int y = playing_field_offset - viewport.y;
				spr->draw_a5(x, y, al_map_rgba(0,101,108,160));

				end_info_bmp();
			}

			debugger->highlight_sprite_id = 0;
		}
	}
	else
	{
		debugger->info_bmp_saved = false;
	}

	return running;
}
