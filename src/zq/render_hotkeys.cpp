#include "zq/render_hotkeys.h"

#include "allegro/keyboard.h"
#include "allegro5/bitmap.h"
#include "allegro5/color.h"
#include "core/fonts.h"
#include "zalleg/render.h"
#include "core/hotkey.h"
#include "core/zdefs.h"
#include "zalleg/zsys.h"
#include "zalleg/zalleg.h"
#include "zq/render.h"
#include "zq/zq_hotkey.h"

#include <cassert>
#include <set>
#include <string>
#include <vector>

// TODO
// - Only shows shortcuts from the Hotkey system, which excludes dialog-specific shortcuts.

namespace {

// Which group each hotkey is drawn under. The groups, and the ids within them, are drawn
// in the order given here; every id must appear in exactly one group (anything missed
// falls into "Misc.").
const std::vector<int> group_action_ids = {
	ZQKEY_NEW, ZQKEY_OPEN, ZQKEY_SAVE, ZQKEY_SAVEAS, ZQKEY_REVERT,
	ZQKEY_UNDO, ZQKEY_REDO, ZQKEY_DELETE, ZQKEY_DELETE_MAP, ZQKEY_APPLY_TEMPLATE_ALL,
	ZQKEY_COMPILE_ZSCRIPT, ZQKEY_COMPILE_QUICK, ZQKEY_COMPILE_SMART, ZQKEY_TESTMODE,
	ZQKEY_RESET_TRANSP, ZQKEY_CSET_FIX, ZQKEY_FIX_WATER_SOLID, ZQKEY_FIX_EFFECT_SQUARE,
};

const std::vector<int> group_toggle_ids = {
	ZQKEY_TOGGLE_DARK, ZQKEY_DARK_PREVIEW, ZQKEY_PREV_MODE,
	ZQKEY_SHOW_FLAGS, ZQKEY_SHOW_FFCS, ZQKEY_SHOW_SOLID, ZQKEY_SHOW_INFO,
	ZQKEY_SHOW_CSETS, ZQKEY_SHOW_TYPES, ZQKEY_SHOW_CMB_CS_INFO,
	ZQKEY_CURR_LAYER_HL, ZQKEY_VIEW_L2_BG, ZQKEY_VIEW_L3_BG, ZQKEY_GRID, ZQKEY_GRID_COLOR,
};

const std::vector<int> group_drawing_mode_ids = {
	ZQKEY_DRAWING_MODE, ZQKEY_DRAWMODE_NORMAL, ZQKEY_DRAWMODE_ALIAS, ZQKEY_DRAWMODE_POOL,
	ZQKEY_DRAWMODE_AUTO,
	ZQKEY_CHANGE_ALIAS_ORIGIN, ZQKEY_CAUTO_HEIGHTMINUS, ZQKEY_CAUTO_HEIGHTPLUS,
};

const std::vector<int> group_selection_ids = {
	ZQKEY_LYR_0, ZQKEY_LYR_1, ZQKEY_LYR_2, ZQKEY_LYR_3, ZQKEY_LYR_4, ZQKEY_LYR_5, ZQKEY_LYR_6,
	ZQKEY_SCR_LPAL_0, ZQKEY_SCR_LPAL_1, ZQKEY_SCR_LPAL_2, ZQKEY_SCR_LPAL_3, ZQKEY_SCR_LPAL_4,
	ZQKEY_SCR_LPAL_5, ZQKEY_SCR_LPAL_6, ZQKEY_SCR_LPAL_7,
	ZQKEY_SCR_LPAL_8, ZQKEY_SCR_LPAL_9, ZQKEY_SCR_LPAL_10, ZQKEY_SCR_LPAL_11, ZQKEY_SCR_LPAL_12,
	ZQKEY_SCR_LPAL_13, ZQKEY_SCR_LPAL_14, ZQKEY_SCR_LPAL_15,
	ZQKEY_MINUS_COLOR, ZQKEY_PLUS_COLOR,
	ZQKEY_MINUS_SCR_PAL, ZQKEY_PLUS_SCR_PAL, ZQKEY_MINUS_16_SCR_PAL, ZQKEY_PLUS_16_SCR_PAL,
	ZQKEY_MINUS_FLAG, ZQKEY_PLUS_FLAG,
	ZQKEY_SQUAREPANEL_UP, ZQKEY_SQUAREPANEL_DOWN,
};

const std::vector<int> group_map_navigation_ids = {
	ZQKEY_MINUS_MAP, ZQKEY_PLUS_MAP, ZQKEY_GOTO_MAP,
	ZQKEY_SCROLL_SCREEN_UP, ZQKEY_SCROLL_SCREEN_DOWN, ZQKEY_SCROLL_SCREEN_LEFT,
	ZQKEY_SCROLL_SCREEN_RIGHT,
	ZQKEY_WARP_SCREEN_UP, ZQKEY_WARP_SCREEN_DOWN, ZQKEY_WARP_SCREEN_LEFT,
	ZQKEY_WARP_SCREEN_RIGHT,
	ZQKEY_BACK, ZQKEY_FORWARD,
	ZQKEY_SCREEN_ZOOM_IN, ZQKEY_SCREEN_ZOOM_OUT, ZQKEY_VIEW_MAP,
};

const std::vector<int> group_combo_navigation_ids = {
	ZQKEY_GOTO_PAGE, ZQKEY_COMBO_PAGEUP, ZQKEY_COMBO_PAGEDN,
	ZQKEY_SCROLL_COMBO_UP, ZQKEY_SCROLL_COMBO_DOWN, ZQKEY_SCROLL_COMBO_LEFT,
	ZQKEY_SCROLL_COMBO_RIGHT,
};

const std::vector<int> group_paste_ids = {
	ZQKEY_COPY, ZQKEY_PASTE, ZQKEY_PASTEALL, ZQKEY_PASTETOALL, ZQKEY_PASTEALLTOALL,
	ZQKEY_PASTE_SCREEN_DATA, ZQKEY_PASTE_ROOMTYPE, ZQKEY_PASTE_GUY_STR, ZQKEY_PASTE_DOORS,
	ZQKEY_PASTE_ENEMIES, ZQKEY_PASTE_FFCS,
	ZQKEY_PASTE_LAYERS, ZQKEY_PASTE_SECRET_COMBOS, ZQKEY_PASTE_UNDERCOMBO, ZQKEY_PASTE_WARPS,
	ZQKEY_PASTE_WARP_RET, ZQKEY_PASTE_PALETTE,
};

const std::vector<int> group_screen_dialog_ids = {
	ZQKEY_SCRDATA, ZQKEY_ROOM, ZQKEY_ENEMIES, ZQKEY_ITEMS, ZQKEY_FLAGS,
	ZQKEY_SECRET_COMBO, ZQKEY_UNDERCOMBO, ZQKEY_DOORS, ZQKEY_LAYERS, ZQKEY_FFCS,
	ZQKEY_TILEWARP, ZQKEY_SIDEWARP, ZQKEY_SCREEN_PAL, ZQKEY_MAZE_PATH,
	ZQKEY_EDIT_SCREEN_SCRIPT, ZQKEY_NES_DUNGEON_TEMPL, ZQKEY_SCREEN_NOTES,
	ZQKEY_BROWSE_SCREEN_NOTES,
};

const std::vector<int> group_editor_ids = {
	ZQKEY_COMBOS, ZQKEY_TILES, ZQKEY_ALIASES, ZQKEY_CMBPOOLS, ZQKEY_AUTOCMB,
	ZQKEY_DMAPS, ZQKEY_MAPS, ZQKEY_MAP_STYLES, ZQKEY_STRINGS, ZQKEY_ENDSTRING,
	ZQKEY_EDIT_ENEMIES, ZQKEY_EDIT_ITEMS, ZQKEY_EDIT_DROPSETS, ZQKEY_EDIT_SPRITE_DATA,
	ZQKEY_EDIT_SUBSCREENS,
	ZQKEY_HERO_SPRITE, ZQKEY_GAME_ICONS, ZQKEY_MCGUFFIN_PIECES, ZQKEY_SHOP_TYPES,
	ZQKEY_INFO_TYPES,
	ZQKEY_BOTTLE_TYPES, ZQKEY_BOTTLE_SHOP_TYPES, ZQKEY_DOOR_COMBO_SETS, ZQKEY_WARP_RINGS,
	ZQKEY_MIDIS, ZQKEY_SFX, ZQKEY_HEADER, ZQKEY_INIT_DATA, ZQKEY_CHEATS,
	ZQKEY_MISC_COLORS, ZQKEY_PALETTES_MAIN, ZQKEY_PALETTES_LEVEL, ZQKEY_PALETTES_SPRITES,
	ZQKEY_VIEW_PALETTE, ZQKEY_VIEW_PIC,
};

const std::vector<int> group_default_ids = {
	ZQKEY_DEFAULT_COMBOS, ZQKEY_DEFAULT_TILES, ZQKEY_DEFAULT_ENEMIES, ZQKEY_DEFAULT_ITEMS,
	ZQKEY_DEFAULT_SPRITE_DATA, ZQKEY_DEFAULT_PALETTES, ZQKEY_DEFAULT_MAP_STYLES,
};

const std::vector<int> group_quest_rule_ids = {
	ZQKEY_QUEST_RULES, ZQKEY_RULES_ZSCRIPT, ZQKEY_RULES_COMPILER, ZQKEY_SEARCH_QRS,
	ZQKEY_RULESETS, ZQKEY_RULETMPLS,
};

const std::vector<int> group_import_export_ids = {
	ZQKEY_IMPORT_COMBOS, ZQKEY_EXPORT_COMBOS, ZQKEY_IMPORT_DMAPS, ZQKEY_EXPORT_DMAPS,
	ZQKEY_IMPORT_ZGP, ZQKEY_EXPORT_ZGP,
	ZQKEY_IMPORT_MAP, ZQKEY_EXPORT_MAP, ZQKEY_IMPORT_PALS, ZQKEY_EXPORT_PALS,
	ZQKEY_IMPORT_STRINGS, ZQKEY_EXPORT_STRINGS,
	ZQKEY_IMPORT_TILES, ZQKEY_EXPORT_TILES,
};

const std::vector<int> group_report_ids = {
	ZQKEY_INTEG_CHECK_ALL, ZQKEY_INTEG_CHECK_SCREENS, ZQKEY_INTEG_CHECK_WARPS,
	ZQKEY_REPORT_COMBO_LOC, ZQKEY_REPORT_TYPE_LOC, ZQKEY_REPORT_ENEMY_LOC,
	ZQKEY_REPORT_ITEM_LOC,
	ZQKEY_REPORT_SCRIPT_LOC, ZQKEY_REPORT_LINKS_HERE, ZQKEY_USED_COMBOS, ZQKEY_BUGGY_NEXT,
};

const std::vector<int> group_program_ids = {
	ZQKEY_FULLSCREEN, ZQKEY_VIDEO_MODE, ZQKEY_SCREENSHOT, ZQKEY_SCREEN_SCREENSHOT,
	ZQKEY_DEBUG_CONSOLE,
	ZQKEY_PLAY_MUSIC, ZQKEY_AMBIENT_MUSIC, ZQKEY_CHANGE_TRACK, ZQKEY_STOP_TUNES,
	ZQKEY_OPTIONS, ZQKEY_CLEAR_QST_PATH, ZQKEY_SHOW_HOTKEYS, ZQKEY_BIND_HOTKEYS,
	ZQKEY_SAVE_MENUS, ZQKEY_ABOUT, ZQKEY_EXIT,
};

struct HotkeyGroup
{
	const char* name;
	std::vector<int> ids;
};

// The groups, in draw order (as are the ids within each group).
// Built once: `layout` runs several times per frame while the panel is open.
const std::vector<HotkeyGroup>& get_hotkey_groups()
{
	static const std::vector<HotkeyGroup> groups = []() {
		std::vector<HotkeyGroup> groups = {
			{"Actions", group_action_ids},
			{"Toggles", group_toggle_ids},
			{"Drawing Modes", group_drawing_mode_ids},
			{"Selection", group_selection_ids},
			{"Map Navigation", group_map_navigation_ids},
			{"Combo Navigation", group_combo_navigation_ids},
			{"Copy/Paste", group_paste_ids},
			{"Screen Dialogs", group_screen_dialog_ids},
			{"Editors", group_editor_ids},
			{"Defaults", group_default_ids},
			{"Quest Rules", group_quest_rule_ids},
			{"Import/Export", group_import_export_ids},
			{"Reports", group_report_ids},
			{"Program", group_program_ids},
			{"Misc.", {}},
		};

		std::set<int> categorized;
		for (const auto& group : groups)
		{
			for (int hotkey_index : group.ids)
			{
				[[maybe_unused]] bool inserted = categorized.insert(hotkey_index).second;
				// Each hotkey belongs to exactly one group.
				assert(inserted);
			}
		}

		// Anything not categorized above.
		auto& misc_ids = groups.back().ids;
		for (int i = ZQKEY_UNDO; i < ZQKEY_MAX; i++)
		{
			if (!categorized.contains(i))
				misc_ids.push_back(i);
		}

		return groups;
	}();
	return groups;
}

// Layers and screen palettes are one hotkey per index; a row each would be 23 rows of
// near-identical text. A run of consecutive indices bound to consecutive keys with the
// same modifiers is drawn as a single row instead ("Edit Layer 0-6" / "0-6"); a binding
// that does not continue the run starts a new row, so nothing is hidden.
const std::vector<std::pair<int, int>> collapsible_ranges = {
	{ZQKEY_LYR_0, ZQKEY_LYR_6},
	{ZQKEY_SCR_LPAL_0, ZQKEY_SCR_LPAL_15},
};

// One row of the cheatsheet: a hotkey name and its (up to two) bindings.
struct HotkeyRow
{
	std::string name;
	std::string keys[2];
};

// Whether `b` continues `a`'s run: the same modifiers and the next key, in both slots.
bool continues_binding_run(const Hotkey& a, const Hotkey& b)
{
	for (int i = 0; i < 2; i++)
	{
		if (a.modflag[i] != b.modflag[i])
			return false;

		// Both slots unbound also continues the run. Only one bound never does.
		if (a.hotkey[i] || b.hotkey[i])
		{
			if (!a.hotkey[i] || !b.hotkey[i] || b.hotkey[i] != a.hotkey[i] + 1)
				return false;
		}
	}

	return true;
}

// Collapses the two ends of a run into one label: "Edit Layer 0" and "Edit Layer 6"
// become "Edit Layer 0-6"; "Ctrl+Num 0" and "Ctrl+Num 5" become "Ctrl+Num 0-5".
std::string join_range(const std::string& first, const std::string& last)
{
	size_t i = 0;
	while (i < first.size() && i < last.size() && first[i] == last[i])
		i++;
	if (i == last.size())
		return first;
	// When `first` is a strict prefix of `last` ("F1" / "F10"), trimming would leave a
	// misleading suffix ("F1-0"); keep the full label.
	if (i == first.size())
		return first + "-" + last;
	return first + "-" + last.substr(i);
}

// Builds the row starting at `ids[i]`, advancing `i` past any hotkeys collapsed into it.
// Returns false when the row has nothing to show (no bindings).
bool build_row(const std::vector<int>& ids, size_t& i, HotkeyRow& row)
{
	int first = ids[i];
	int range_last = 0;
	for (auto [id_first, id_last] : collapsible_ranges)
	{
		if (first >= id_first && first <= id_last)
			range_last = id_last;
	}

	size_t end = i;
	while (ids[end] < range_last && end + 1 < ids.size() && ids[end + 1] == ids[end] + 1 &&
		continues_binding_run(zq_hotkeys[ids[end]], zq_hotkeys[ids[end] + 1]))
		end++;
	int last = ids[end];
	i = end;

	const auto& hotkey = zq_hotkeys[first];
	row = {};
	row.name = get_hotkey_name(first);
	if (last > first)
		row.name = join_range(row.name, get_hotkey_name(last));

	int num_keys = 0;
	for (int q = 0; q < 2; q++)
	{
		if (!hotkey.hotkey[q])
			continue;
		row.keys[num_keys] = hotkey.get_name(q);
		if (last > first)
			row.keys[num_keys] = join_range(row.keys[num_keys], zq_hotkeys[last].get_name(q));
		num_keys++;
	}
	return num_keys > 0;
}

ALLEGRO_COLOR hex(unsigned int h)
{
	int r = (h>>16) & 0xff;
	int g = (h>>8) & 0xff;
	int b = (h) & 0xff;
	return al_map_rgb(r, g, b);
}

// Dims the whole screen behind the panel: a single dark pixel, stretched over the
// window by its transform.
RenderTreeItem rti_hotkeys_backdrop("hot_keys_backdrop");

class HotKeysRTI : public RenderTreeItem
{
public:
	HotKeysRTI(std::string name) : RenderTreeItem(name) {};

private:
	static constexpr int colwidth = 280;
	static constexpr int panel_padding = 10;
	static constexpr int group_margin = 10;
	static constexpr int entry_padding = 5;
	static constexpr int window_margin = 15;
	static constexpr int title_font_size = 2;
	static constexpr int entry_font_size = 1;

	// The virtual box height the final measure used.
	int layout_box_h = 0;

	// Fit the panel to the window each frame: measure the content, and when it is too
	// wide to fit, uniformly scale the panel down via the transform instead of clipping
	// it. (A scroll container is a poor fit here - the overlay dismisses on any key
	// release, so there is nothing to scroll it with.) Content flows into columns up to
	// the given height, so a smaller scale also means a taller virtual box and fewer
	// columns; step the scale down until the columns fit.
	void prepare()
	{
		auto* root = get_root_rti();
		int avail_w = root->width - 2*window_margin;
		int avail_h = root->height - 2*window_margin;
		if (avail_w <= 0 || avail_h <= 0)
			return;

		float scale = 1;
		layout_box_h = avail_h;
		auto size = layout(nullptr, layout_box_h);
		while (size.first * scale > avail_w && scale > 0.35)
		{
			scale *= 0.85;
			layout_box_h = (int)(avail_h / scale);
			size = layout(nullptr, layout_box_h);
		}
		auto [w, h] = size;

		if (w != width || h != height)
		{
			set_size(w, h);
			dirty = true;
		}
		set_transform({
			.x = (float)((int)(root->width - w*scale) / 2),
			.y = (float)((int)(root->height - h*scale) / 2),
			.xscale = scale,
			.yscale = scale,
		});
		// Linear filtering, for legibility when the panel is scaled down.
		bitmap_flags = get_bitmap_create_flags(true) | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR;

		rti_hotkeys_backdrop.set_transform({.xscale = (float)root->width, .yscale = (float)root->height});
	}

	void render([[maybe_unused]] bool bitmap_resized)
	{
		al_draw_filled_rectangle(0, 0, al_get_bitmap_width(bitmap), al_get_bitmap_height(bitmap), hex(0x22272f));
		layout(bitmap, layout_box_h);
	}

	// Lays out the hotkey groups in columns of the given height, drawing to `dest` -
	// or, when `dest` is null, only measuring. Returns the used (width, height).
	std::pair<int, int> layout(ALLEGRO_BITMAP* dest, int box_h)
	{
		auto color_text = hex(0xadbac7);
		auto color_entry_text = hex(0xdddd00);
		auto color_bg_secondary = hex(0x2d333b);

		ALLEGRO_FONT* font = get_zc_font_a5(font_lfont_l);

		int height = box_h - panel_padding;
		int x = panel_padding;
		int y = panel_padding;
		int max_y = 0;
		int line_height = al_get_font_line_height(font);
		const char* cont_name = nullptr;
		auto draw_group_header = [&](const std::string& text) {
			if (dest)
			{
				al_draw_filled_rectangle(x, y, x + colwidth, y + line_height*title_font_size, color_bg_secondary);
				render_text(dest, font, text, x, y, title_font_size, color_text);
			}
			y += line_height*title_font_size;
			max_y = std::max(max_y, y);
		};
		auto ensure_space = [&](int vspace) {
			if (y + vspace >= height)
			{
				y = panel_padding;
				x += colwidth + group_margin;
				// A group spilling into the next column repeats its header.
				if (cont_name)
					draw_group_header(std::string(cont_name) + " (cont.)");
			}
		};

		for (const auto& group : get_hotkey_groups())
		{
			bool has_any = false;
			for (int hotkey_index : group.ids)
			{
				if (zq_hotkeys[hotkey_index].hotkey[0] || zq_hotkeys[hotkey_index].hotkey[1])
				{
					has_any = true;
					break;
				}
			}
			if (!has_any) continue;

			cont_name = nullptr;
			// Require space for at least three entries, else break to next column.
			ensure_space(line_height*title_font_size + 3*line_height*entry_font_size);
			draw_group_header(group.name);
			cont_name = group.name;

			for (size_t i = 0; i < group.ids.size(); i++)
			{
				HotkeyRow row;
				if (!build_row(group.ids, i, row))
					continue;

				int num_lines = row.keys[1].empty() ? 1 : 2;
				ensure_space(num_lines*line_height*entry_font_size + 1);
				if (dest)
					render_text(dest, font, row.name, x + entry_padding, y, entry_font_size, color_text);
				for (int q = 0; q < num_lines; q++)
				{
					if (dest)
					{
						const std::string& keys = row.keys[q];
						int end_col_x = x + colwidth - entry_padding - al_get_text_width(font, keys.c_str()) * entry_font_size;
						render_text(dest, font, keys, end_col_x, y, entry_font_size, color_entry_text);
					}
					y += line_height*entry_font_size;
				}
				if (dest)
					al_draw_line(x, y, x + colwidth, y, color_bg_secondary, 1);
				y += 1;
				max_y = std::max(max_y, y);
			}

			y += group_margin;
		}

		return {x + colwidth + panel_padding, std::min(box_h, max_y + panel_padding)};
	}
};

HotKeysRTI rti_hotkeys("hot_keys");
bool is_active = false;

} // namespace

void hotkeys_run()
{
	auto parent = get_root_rti();
	rti_hotkeys_backdrop.set_size(1, 1);
	rti_hotkeys_backdrop.render_cb = [](RenderTreeItem*, bool) {
		al_draw_filled_rectangle(0, 0, 1, 1, al_map_rgba_f(0, 0, 0, 0.6));
	};
	parent->add_child(&rti_hotkeys_backdrop);
	parent->add_child(&rti_hotkeys);

	zq_freeze_all_rti();
	rti_hotkeys.freeze = false;
	rti_hotkeys_backdrop.freeze = false;

	zalleg_wait_for_all_keys_up();

	is_active = true;
	keyboard_lowlevel_callback = [](int scancode) {
		// any key release
		if (scancode & 0x80)
			is_active = false;
	};

	while (is_active)
	{
		if (close_button_quit)
			break;
		if (gui_mouse_b())
			break;

		poll_keyboard();
		update_hw_screen();
	}

	is_active = false;
	keyboard_lowlevel_callback = nullptr;

	clear_keybuf();
	rti_hotkeys.remove();
	rti_hotkeys_backdrop.remove();
}

void hotkeys_invalidate()
{
	rti_hotkeys.dirty = true;
}
