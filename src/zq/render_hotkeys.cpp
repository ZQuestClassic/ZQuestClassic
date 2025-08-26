#include "zq/render_hotkeys.h"

#include "allegro5/bitmap.h"
#include "allegro5/color.h"
#include "base/fonts.h"
#include "base/render.h"
#include "base/hotkey.h"
#include "zq/render.h"
#include "zq/zq_hotkey.h"

#include <string>
#include <vector>
#include <set>

// TODO
// - Only shows shortcuts from the Hotkey system, which excludes dialog-specific shortcuts.
// - If too many hotkeys are configured, they won't fit on screen. Need a scroll container.
// - Refactor building block RTIs for CSS box model-like behavior (margin, padding), and common drawing stuff
//   (bg color, border, etc). And a text RTI component.

// Repetitive.
const std::set<int> group_skip_ids = {
	ZQKEY_LYR_1,
	ZQKEY_LYR_2,
	ZQKEY_LYR_3,
	ZQKEY_LYR_4,
	ZQKEY_LYR_5,
	ZQKEY_LYR_6,
	ZQKEY_SCR_LPAL_1,
	ZQKEY_SCR_LPAL_2,
	ZQKEY_SCR_LPAL_3,
	ZQKEY_SCR_LPAL_4,
	ZQKEY_SCR_LPAL_5,
	ZQKEY_SCR_LPAL_6,
	ZQKEY_SCR_LPAL_7,
	ZQKEY_SCR_LPAL_8,
	ZQKEY_SCR_LPAL_9,
	ZQKEY_SCR_LPAL_10,
	ZQKEY_SCR_LPAL_11,
	ZQKEY_SCR_LPAL_12,
	ZQKEY_SCR_LPAL_13,
	ZQKEY_SCR_LPAL_14,
	ZQKEY_SCR_LPAL_15,
};

const std::set<int> group_action_ids = {
	ZQKEY_SAVE,
	ZQKEY_SAVEAS,
	ZQKEY_OPEN,
	ZQKEY_RESET_TRANSP,
	ZQKEY_COMPILE_ZSCRIPT,
	ZQKEY_COMPILE_SMART,
	ZQKEY_SCREENSHOT,
	ZQKEY_FULLSCREEN,
	ZQKEY_TESTMODE,
	ZQKEY_EXIT,
	ZQKEY_CHANGE_TRACK,
	ZQKEY_NEW,
	ZQKEY_PLAY_MUSIC,
	ZQKEY_DELETE,
	ZQKEY_DELETE_MAP,
	ZQKEY_USED_COMBOS,
	ZQKEY_COMPILE_QUICK,
	ZQKEY_SCREEN_SCREENSHOT,
	ZQKEY_APPLY_TEMPLATE_ALL,
	ZQKEY_CSET_FIX,
	ZQKEY_FIX_WATER_SOLID,
	ZQKEY_FIX_EFFECT_SQUARE,
	ZQKEY_VIDEO_MODE,
	ZQKEY_REVERT,
	ZQKEY_VIEW_MAP,
	ZQKEY_CLEAR_QST_PATH,
	ZQKEY_UNDO,
	ZQKEY_REDO,
};

const std::set<int> group_toggle_ids = {
	ZQKEY_TOGGLE_DARK,
	ZQKEY_DEBUG_CONSOLE,
	ZQKEY_SHOW_FLAGS,
	ZQKEY_SHOW_FFCS,
	ZQKEY_DARK_PREVIEW,
	ZQKEY_DRAWING_MODE,
	ZQKEY_SHOW_SOLID,
	ZQKEY_PREV_MODE,
	ZQKEY_MINUS_COLOR,
	ZQKEY_PLUS_COLOR,
	ZQKEY_MINUS_SCR_PAL,
	ZQKEY_PLUS_SCR_PAL,
	ZQKEY_MINUS_16_SCR_PAL,
	ZQKEY_PLUS_16_SCR_PAL,
	ZQKEY_LYR_0,
	ZQKEY_SCR_LPAL_0,
	ZQKEY_DRAWMODE_NORMAL,
	ZQKEY_DRAWMODE_ALIAS,
	ZQKEY_DRAWMODE_POOL,
	ZQKEY_DRAWMODE_AUTO,
	ZQKEY_VIEW_L2_BG,
	ZQKEY_VIEW_L3_BG,
	ZQKEY_GRID,
	ZQKEY_GRID_COLOR,
	ZQKEY_CHANGE_ALIAS_ORIGIN,
	ZQKEY_CAUTO_HEIGHTPLUS,
	ZQKEY_CAUTO_HEIGHTMINUS,
	ZQKEY_CURR_LAYER_HL,
	ZQKEY_MINUS_FLAG,
	ZQKEY_PLUS_FLAG,
	ZQKEY_SQUAREPANEL_UP,
	ZQKEY_SQUAREPANEL_DOWN,
};

const std::set<int> group_map_navigation_ids = {
	ZQKEY_MINUS_MAP,
	ZQKEY_PLUS_MAP,
	ZQKEY_SCREEN_ZOOM_IN,
	ZQKEY_SCREEN_ZOOM_OUT,
	ZQKEY_SCROLL_SCREEN_UP,
	ZQKEY_SCROLL_SCREEN_DOWN,
	ZQKEY_SCROLL_SCREEN_LEFT,
	ZQKEY_SCROLL_SCREEN_RIGHT,
	ZQKEY_WARP_SCREEN_UP,
	ZQKEY_WARP_SCREEN_DOWN,
	ZQKEY_WARP_SCREEN_LEFT,
	ZQKEY_WARP_SCREEN_RIGHT,
	ZQKEY_GOTO_MAP,
};

const std::set<int> group_combo_navigation_ids = {
	ZQKEY_GOTO_PAGE,
	ZQKEY_COMBO_PAGEUP,
	ZQKEY_COMBO_PAGEDN,
	ZQKEY_SCROLL_COMBO_UP,
	ZQKEY_SCROLL_COMBO_DOWN,
	ZQKEY_SCROLL_COMBO_LEFT,
	ZQKEY_SCROLL_COMBO_RIGHT,
};

const std::set<int> group_dialog_ids = {
	ZQKEY_SCREEN_PAL,
	ZQKEY_SECRET_COMBO,
	ZQKEY_DOORS,
	ZQKEY_FFCS,
	ZQKEY_FLAGS,
	ZQKEY_SCRDATA,
	ZQKEY_SHOW_CMB_CS_INFO,
	ZQKEY_TILEWARP,
	ZQKEY_SIDEWARP,
	ZQKEY_LAYERS,
	ZQKEY_ENEMIES,
	ZQKEY_ITEMS,
	ZQKEY_COMBOS,
	ZQKEY_SHOW_INFO,
	ZQKEY_STRINGS,
	ZQKEY_ROOM,
	ZQKEY_TILES,
	ZQKEY_ABOUT,
	ZQKEY_CHEATS,
	ZQKEY_ALIASES,
	ZQKEY_CMBPOOLS,
	ZQKEY_AUTOCMB,
	ZQKEY_DEFAULT_COMBOS,
	ZQKEY_DMAPS,
	ZQKEY_MAPS,
	ZQKEY_DOOR_COMBO_SETS,
	ZQKEY_ENDSTRING,
	ZQKEY_EDIT_ENEMIES,
	ZQKEY_DEFAULT_ENEMIES,
	ZQKEY_GAME_ICONS,
	ZQKEY_HEADER,
	ZQKEY_INFO_TYPES,
	ZQKEY_INIT_DATA,
	ZQKEY_EDIT_ITEMS,
	ZQKEY_PALETTES_LEVEL,
	ZQKEY_HERO_SPRITE,
	ZQKEY_PALETTES_MAIN,
	ZQKEY_DEFAULT_MAP_STYLES,
	ZQKEY_MAP_STYLES,
	ZQKEY_MIDIS,
	ZQKEY_MISC_COLORS,
	ZQKEY_OPTIONS,
	ZQKEY_DEFAULT_PALETTES,
	ZQKEY_MAZE_PATH,
	ZQKEY_SFX,
	ZQKEY_SHOP_TYPES,
	ZQKEY_PALETTES_SPRITES,
	ZQKEY_DEFAULT_SPRITE_DATA,
	ZQKEY_STOP_TUNES,
	ZQKEY_EDIT_SUBSCREENS,
	ZQKEY_AMBIENT_MUSIC,
	ZQKEY_NES_DUNGEON_TEMPL,
	ZQKEY_DEFAULT_TILES,
	ZQKEY_MCGUFFIN_PIECES,
	ZQKEY_UNDERCOMBO,
	ZQKEY_VIEW_PALETTE,
	ZQKEY_VIEW_PIC,
	ZQKEY_WARP_RINGS,
	ZQKEY_EDIT_SPRITE_DATA,
	ZQKEY_SHOW_CSETS,
	ZQKEY_SHOW_TYPES,
	ZQKEY_DEFAULT_ITEMS,
	ZQKEY_EDIT_DROPSETS,
	ZQKEY_QUEST_RULES,
	ZQKEY_RULES_ZSCRIPT,
	ZQKEY_RULES_COMPILER,
	ZQKEY_EDIT_SCREEN_SCRIPT,
	ZQKEY_BOTTLE_TYPES,
	ZQKEY_BOTTLE_SHOP_TYPES,
	ZQKEY_SEARCH_QRS,
	ZQKEY_RULESETS,
	ZQKEY_RULETMPLS,
	ZQKEY_BIND_HOTKEYS,
};

const std::set<int> group_import_export_ids = {
	ZQKEY_IMPORT_COMBOS,
	ZQKEY_EXPORT_COMBOS,
	ZQKEY_IMPORT_DMAPS,
	ZQKEY_EXPORT_DMAPS,
	ZQKEY_IMPORT_ZGP,
	ZQKEY_EXPORT_ZGP,
	ZQKEY_IMPORT_MAP,
	ZQKEY_EXPORT_MAP,
	ZQKEY_IMPORT_PALS,
	ZQKEY_EXPORT_PALS,
	ZQKEY_IMPORT_STRINGS,
	ZQKEY_EXPORT_STRINGS,
	ZQKEY_IMPORT_TILES,
	ZQKEY_EXPORT_TILES,
};

const std::set<int> group_paste_ids = {
	ZQKEY_COPY,
	ZQKEY_PASTE,
	ZQKEY_PASTEALL,
	ZQKEY_PASTETOALL,
	ZQKEY_PASTEALLTOALL,
	ZQKEY_PASTE_DOORS,
	ZQKEY_PASTE_ENEMIES,
	ZQKEY_PASTE_FFCS,
	ZQKEY_PASTE_GUY_STR,
	ZQKEY_PASTE_LAYERS,
	ZQKEY_PASTE_ROOMTYPE,
	ZQKEY_PASTE_SCREEN_DATA,
	ZQKEY_PASTE_SECRET_COMBOS,
	ZQKEY_PASTE_UNDERCOMBO,
	ZQKEY_PASTE_WARP_RET,
	ZQKEY_PASTE_WARPS,
	ZQKEY_PASTE_PALETTE,
};

const std::set<int> group_report_ids = {
	ZQKEY_INTEG_CHECK_ALL,
	ZQKEY_INTEG_CHECK_SCREENS,
	ZQKEY_INTEG_CHECK_WARPS,
	ZQKEY_REPORT_COMBO_LOC,
	ZQKEY_REPORT_TYPE_LOC,
	ZQKEY_REPORT_ENEMY_LOC,
	ZQKEY_REPORT_ITEM_LOC,
	ZQKEY_REPORT_SCRIPT_LOC,
	ZQKEY_REPORT_LINKS_HERE,
	ZQKEY_BUGGY_NEXT,
};

static ALLEGRO_COLOR hex(unsigned int h)
{
	int r = (h>>16) & 0xff;
	int g = (h>>8) & 0xff;
	int b = (h) & 0xff;
	return al_map_rgb(r, g, b);
}

class HotKeysRTI : public RenderTreeItem
{
public:
	HotKeysRTI(std::string name) : RenderTreeItem(name) {};

private:
	void render(bool bitmap_resized)
	{
		int num_columns = 3;
		auto color_text = hex(0xadbac7);
		auto color_entry_text = hex(0xdddd00);
		auto color_bg = hex(0x22272f);
		auto color_bg_secondary = hex(0x2d333b);

		ALLEGRO_FONT* font = get_zc_font_a5(font_lfont_l);

		std::set<int> group_misc_ids;
		for (int i = ZQKEY_UNDO; i < ZQKEY_MAX; i++)
		{
			if (group_skip_ids.contains(i)) continue;
			if (group_action_ids.contains(i)) continue;
			if (group_toggle_ids.contains(i)) continue;
			if (group_map_navigation_ids.contains(i)) continue;
			if (group_combo_navigation_ids.contains(i)) continue;
			if (group_dialog_ids.contains(i)) continue;
			if (group_import_export_ids.contains(i)) continue;
			if (group_paste_ids.contains(i)) continue;
			if (group_report_ids.contains(i)) continue;

			group_misc_ids.insert(i);
		}

		// Vector of (group name, hotkeys).
		std::vector<std::pair<std::string, const std::set<int>&>> groups = {
			{"Actions", group_action_ids},
			{"Toggles", group_toggle_ids},
			{"Map Navigation", group_map_navigation_ids},
			{"Combo Navigation", group_combo_navigation_ids},
			{"Dialogs", group_dialog_ids},
			{"Import/Export", group_import_export_ids},
			{"Copy/Paste", group_paste_ids},
			{"Reports", group_report_ids},
			{"Misc.", group_misc_ids},
		};

		int padding = 0;
		int margin = 15;
		int group_margin = 10;
		int entry_padding = 5;
		int left = padding + margin;
		int top = padding + margin;
		int right = al_get_bitmap_width(bitmap) - padding - margin;
		int bottom = al_get_bitmap_height(bitmap) - padding - margin;
		int width = right - left;
		int height = bottom - top;
		int title_font_size = 2;
		int entry_font_size = 1;

		int x = left;
		int y = top;
		int line_height = al_get_font_line_height(font);
		int colwidth = (width - group_margin*(num_columns+1)) / num_columns;
		auto ensure_space = [&](int vspace) {
			if (y + vspace >= height)
			{
				y = top;
				x += colwidth + group_margin;
			}
		};

		al_draw_filled_rectangle(0, 0, al_get_bitmap_width(bitmap), al_get_bitmap_height(bitmap), al_map_rgba_f(0, 0, 0, 0.6));
		al_draw_filled_rectangle(left, top, right, bottom, color_bg);

		for (const auto& group : groups)
		{
			bool has_any = false;
			for (int hotkey_index : group.second)
			{
				if (zq_hotkeys[hotkey_index].hotkey[0] || zq_hotkeys[hotkey_index].hotkey[1])
				{
					has_any = true;
					break;
				}
			}
			if (!has_any) continue;

			// Require space for at least three entries, else break to next column.
			ensure_space(line_height*title_font_size + 3*line_height*entry_font_size);
			al_draw_filled_rectangle(x, y, x + colwidth, y + line_height*title_font_size, color_bg_secondary);
			render_text(bitmap, font, group.first, x, y, title_font_size, color_text);
			y += line_height*title_font_size;

			for (int hotkey_index : group.second)
			{
				const auto& hotkey = zq_hotkeys[hotkey_index];
				std::string hk_name = get_hotkey_name(hotkey_index);
				std::string hk1, hk2;
				if (hotkey.hotkey[0]) hk1 = hotkey.get_name(0);
				if (hotkey.hotkey[1])
				{
					if (hotkey.hotkey[0])
						hk2 = hotkey.get_name(1);
					else
						hk1 = hotkey.get_name(1);
				}
				if (hk1.empty())
					continue;

				ensure_space(2*line_height*entry_font_size + 1);
				if (!hk1.empty())
				{
					ensure_space(line_height*entry_font_size);
					int end_col_x = x + colwidth - entry_padding - al_get_text_width(font, hk1.c_str()) * entry_font_size;
					render_text(bitmap, font, hk_name, x + entry_padding, y, entry_font_size, color_text);
					render_text(bitmap, font, hk1, end_col_x, y, entry_font_size, color_entry_text);
					y += line_height*entry_font_size;
				}
				if (!hk2.empty())
				{
					ensure_space(line_height*entry_font_size);
					int end_col_x = x + colwidth - entry_padding - al_get_text_width(font, hk2.c_str()) * entry_font_size;
					render_text(bitmap, font, hk2, end_col_x, y, entry_font_size, color_entry_text);
					y += line_height*entry_font_size;
				}
				al_draw_line(x, y, x + colwidth, y, color_bg_secondary, 1);
				y += 1;
			}

			y += group_margin;
		}
	}
};

static HotKeysRTI rti_hotkeys("hot_keys");
static bool is_active = false;

void hotkeys_toggle_display(bool show)
{
	is_active = show;

	if (!is_active)
	{
		rti_hotkeys.remove();
		return;
	}

	auto parent = get_screen_rti();
	parent->add_child(&rti_hotkeys);
	rti_hotkeys.set_size(parent->width, parent->height);
}

bool hotkeys_is_active()
{
	return is_active;
}

void hotkeys_invalidate()
{
	rti_hotkeys.dirty = true;
}
