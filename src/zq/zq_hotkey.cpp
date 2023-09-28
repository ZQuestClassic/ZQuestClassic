#include "base/zdefs.h"
#include "base/hotkey.h"
#include "zq/zquest.h"
#include "zq/zq_misc.h"
#include "zq/zq_hotkey.h"
#include "dialog/zqhotkeys.h"

extern int32_t prv_mode;
extern bool placing_flags;

int onSetNewLayer(int newlayer);
int onScreenLPal(int lpal);
int32_t do_OpenQuest();
int32_t onSelectFFCombo();
int onReloadPreview();
int onScrollScreen(int dir, bool warp);
int32_t onComboColLeft();
int32_t onComboColRight();
int32_t onComboColUp();
int32_t onComboColDown();
void cycle_compact_sqr(bool down);
void call_testqst_dialog();

char const* get_hotkey_name(int hkey)
{
	switch(hkey)
	{
		case ZQKEY_UNDO: return "Undo";
		case ZQKEY_REDO: return "Redo";
		case ZQKEY_PLUS_FLAG: return "Increment Flag";
		case ZQKEY_MINUS_FLAG: return "Decrement Flag";
		case ZQKEY_SAVE: return "Save";
		case ZQKEY_SAVEAS: return "Save As";
		case ZQKEY_OPEN: return "Open Quest";
		case ZQKEY_SCREEN_PAL: return "Screen Palette";
		case ZQKEY_SECRET_COMBO: return "Secret Combos";
		case ZQKEY_DOORS: return "Doors";
		case ZQKEY_FFCS: return "Freeform Combos";
		case ZQKEY_FLAGS: return "Combo Flags";
		case ZQKEY_SCRDATA: return "Screen Data";
		case ZQKEY_TILEWARP: return "Tile Warps";
		case ZQKEY_SIDEWARP: return "Side Warps";
		case ZQKEY_LAYERS: return "Layers";
		case ZQKEY_RESET_TRANSP: return "Reset Transparency";
		case ZQKEY_COPY: return "Copy Screen";
		case ZQKEY_TOGGLE_DARK: return "Toggle Screen Darkness";
		case ZQKEY_ENEMIES: return "Screen Enemies";
		case ZQKEY_SHOW_FLAGS: return "Show Flags";
		case ZQKEY_ITEMS: return "Screen Item";
		case ZQKEY_COMBOS: return "Combo Pages";
		case ZQKEY_DARK_PREVIEW: return "Preview Darkness";
		case ZQKEY_SHOW_INFO: return "Show Screen Info";
		case ZQKEY_DRAWING_MODE: return "Cycle Drawing Mode";
		case ZQKEY_GOTO_PAGE: return "Goto Combo Page";
		case ZQKEY_SHOW_CMB_CS_INFO: return "Cycle Combo Info";
		case ZQKEY_STRINGS: return "String Editor";
		case ZQKEY_ROOM: return "Room Data";
		case ZQKEY_TILES: return "Tile Pages";
		case ZQKEY_PASTE: return "Paste Screen";
		case ZQKEY_PASTEALL: return "Paste All";
		case ZQKEY_PASTETOALL: return "Paste To All";
		case ZQKEY_PASTEALLTOALL: return "Paste All To All";
		case ZQKEY_SHOW_SOLID: return "Show Walkability";
		case ZQKEY_PREV_MODE: return "Preview Mode";
		case ZQKEY_COMPILE_ZSCRIPT: return "Compile ZScript";
		case ZQKEY_SCREENSHOT: return "Snapshot";
		case ZQKEY_ABOUT: return "About ZQ";
		case ZQKEY_MINUS_MAP: return "Decrement Map";
		case ZQKEY_PLUS_MAP: return "Increment Map";
		case ZQKEY_MINUS_COLOR: return "Decrement CSet";
		case ZQKEY_PLUS_COLOR: return "Increment CSet";
		case ZQKEY_MINUS_SCR_PAL: return "Decrement Screen Palette";
		case ZQKEY_PLUS_SCR_PAL: return "Increment Screen Palette";
		case ZQKEY_MINUS_16_SCR_PAL: return "Decrement Screen Palette x16";
		case ZQKEY_PLUS_16_SCR_PAL: return "Increment Screen Palette x16";
		case ZQKEY_GRID: return "Show Grid";
		case ZQKEY_GRID_COLOR: return "Cycle Grid Color";
		case ZQKEY_COMBO_COL_MODE: return "Toggle Combo Column Mode";
		case ZQKEY_DELETE: return "Delete Screen";
		case ZQKEY_FULLSCREEN: return "Toggle Fullscreen";
		case ZQKEY_LYR_0: return "Edit Layer 0";
		case ZQKEY_LYR_1: return "Edit Layer 1";
		case ZQKEY_LYR_2: return "Edit Layer 2";
		case ZQKEY_LYR_3: return "Edit Layer 3";
		case ZQKEY_LYR_4: return "Edit Layer 4";
		case ZQKEY_LYR_5: return "Edit Layer 5";
		case ZQKEY_LYR_6: return "Edit Layer 6";
		case ZQKEY_SCR_LPAL_0: return "Set Screen Palette: 0";
		case ZQKEY_SCR_LPAL_1: return "Set Screen Palette: 1";
		case ZQKEY_SCR_LPAL_2: return "Set Screen Palette: 2";
		case ZQKEY_SCR_LPAL_3: return "Set Screen Palette: 3";
		case ZQKEY_SCR_LPAL_4: return "Set Screen Palette: 4";
		case ZQKEY_SCR_LPAL_5: return "Set Screen Palette: 5";
		case ZQKEY_SCR_LPAL_6: return "Set Screen Palette: 6";
		case ZQKEY_SCR_LPAL_7: return "Set Screen Palette: 7";
		case ZQKEY_SCR_LPAL_8: return "Set Screen Palette: 8";
		case ZQKEY_SCR_LPAL_9: return "Set Screen Palette: 9";
		case ZQKEY_SCR_LPAL_10: return "Set Screen Palette: A";
		case ZQKEY_SCR_LPAL_11: return "Set Screen Palette: B";
		case ZQKEY_SCR_LPAL_12: return "Set Screen Palette: C";
		case ZQKEY_SCR_LPAL_13: return "Set Screen Palette: D";
		case ZQKEY_SCR_LPAL_14: return "Set Screen Palette: E";
		case ZQKEY_SCR_LPAL_15: return "Set Screen Palette: F";
		case ZQKEY_SCROLL_SCREEN_UP: return "Screen Up";
		case ZQKEY_SCROLL_SCREEN_DOWN: return "Screen Down";
		case ZQKEY_SCROLL_SCREEN_LEFT: return "Screen Left";
		case ZQKEY_SCROLL_SCREEN_RIGHT: return "Screen Right";
		case ZQKEY_WARP_SCREEN_UP: return "Screen Sidewarp Up";
		case ZQKEY_WARP_SCREEN_DOWN: return "Screen Sidewarp Down";
		case ZQKEY_WARP_SCREEN_LEFT: return "Screen Sidewarp Left";
		case ZQKEY_WARP_SCREEN_RIGHT: return "Screen Sidewarp Right";
		case ZQKEY_SCROLL_COMBO_UP: return "Scroll Combopane Up";
		case ZQKEY_SCROLL_COMBO_DOWN: return "Scroll Combopane Down";
		case ZQKEY_SCROLL_COMBO_LEFT: return "Scroll Combopane Left";
		case ZQKEY_SCROLL_COMBO_RIGHT: return "Scroll Combopane Right";
		case ZQKEY_COMBO_PAGEUP: return "Combopane Page Up";
		case ZQKEY_COMBO_PAGEDN: return "Combopane Page Down";
		case ZQKEY_SQUAREPANEL_UP: return "Compact Squarepanel Up";
		case ZQKEY_SQUAREPANEL_DOWN: return "Compact Squarepanel Down";
		case ZQKEY_TESTMODE: return "Test Quest";
		case ZQKEY_CAUTO_HEIGHTMINUS: return "Decrease Autocombo Height";
		case ZQKEY_CAUTO_HEIGHTPLUS: return "Increase Autocombo Height";
		case ZQKEY_CURR_LAYER_HL: return "Highlight Current Layer";
	}
	return "ZQ_NIL_KEY";
}

char const* get_hotkey_cfg_name(int hkey)
{
	switch(hkey)
	{
		case ZQKEY_UNDO: return "ZQKEY_UNDO";
		case ZQKEY_REDO: return "ZQKEY_REDO";
		case ZQKEY_PLUS_FLAG: return "ZQKEY_PLUS_FLAG";
		case ZQKEY_MINUS_FLAG: return "ZQKEY_MINUS_FLAG";
		case ZQKEY_SAVE: return "ZQKEY_SAVE";
		case ZQKEY_SAVEAS: return "ZQKEY_SAVEAS";
		case ZQKEY_OPEN: return "ZQKEY_OPEN";
		case ZQKEY_SCREEN_PAL: return "ZQKEY_SCREEN_PAL";
		case ZQKEY_SECRET_COMBO: return "ZQKEY_SECRET_COMBO";
		case ZQKEY_DOORS: return "ZQKEY_DOORS";
		case ZQKEY_FFCS: return "ZQKEY_FFCS";
		case ZQKEY_FLAGS: return "ZQKEY_FLAGS";
		case ZQKEY_SCRDATA: return "ZQKEY_SCRDATA";
		case ZQKEY_TILEWARP: return "ZQKEY_TILEWARP";
		case ZQKEY_SIDEWARP: return "ZQKEY_SIDEWARP";
		case ZQKEY_LAYERS: return "ZQKEY_LAYERS";
		case ZQKEY_RESET_TRANSP: return "ZQKEY_RESET_TRANSP";
		case ZQKEY_COPY: return "ZQKEY_COPY";
		case ZQKEY_TOGGLE_DARK: return "ZQKEY_TOGGLE_DARK";
		case ZQKEY_ENEMIES: return "ZQKEY_ENEMIES";
		case ZQKEY_SHOW_FLAGS: return "ZQKEY_SHOW_FLAGS";
		case ZQKEY_ITEMS: return "ZQKEY_ITEMS";
		case ZQKEY_COMBOS: return "ZQKEY_COMBOS";
		case ZQKEY_DARK_PREVIEW: return "ZQKEY_DARK_PREVIEW";
		case ZQKEY_SHOW_INFO: return "ZQKEY_SHOW_INFO";
		case ZQKEY_DRAWING_MODE: return "ZQKEY_DRAWING_MODE";
		case ZQKEY_GOTO_PAGE: return "ZQKEY_GOTO_PAGE";
		case ZQKEY_SHOW_CMB_CS_INFO: return "ZQKEY_SHOW_CMB_CS_INFO";
		case ZQKEY_STRINGS: return "ZQKEY_STRINGS";
		case ZQKEY_ROOM: return "ZQKEY_ROOM";
		case ZQKEY_TILES: return "ZQKEY_TILES";
		case ZQKEY_PASTE: return "ZQKEY_PASTE";
		case ZQKEY_PASTEALL: return "ZQKEY_PASTEALL";
		case ZQKEY_PASTETOALL: return "ZQKEY_PASTETOALL";
		case ZQKEY_PASTEALLTOALL: return "ZQKEY_PASTEALLTOALL";
		case ZQKEY_SHOW_SOLID: return "ZQKEY_SHOW_SOLID";
		case ZQKEY_PREV_MODE: return "ZQKEY_PREV_MODE";
		case ZQKEY_COMPILE_ZSCRIPT: return "ZQKEY_COMPILE_ZSCRIPT";
		case ZQKEY_SCREENSHOT: return "ZQKEY_SCREENSHOT";
		case ZQKEY_ABOUT: return "ZQKEY_ABOUT";
		case ZQKEY_MINUS_MAP: return "ZQKEY_MINUS_MAP";
		case ZQKEY_PLUS_MAP: return "ZQKEY_PLUS_MAP";
		case ZQKEY_MINUS_COLOR: return "ZQKEY_MINUS_COLOR";
		case ZQKEY_PLUS_COLOR: return "ZQKEY_PLUS_COLOR";
		case ZQKEY_MINUS_SCR_PAL: return "ZQKEY_MINUS_SCR_PAL";
		case ZQKEY_PLUS_SCR_PAL: return "ZQKEY_PLUS_SCR_PAL";
		case ZQKEY_MINUS_16_SCR_PAL: return "ZQKEY_MINUS_16_SCR_PAL";
		case ZQKEY_PLUS_16_SCR_PAL: return "ZQKEY_PLUS_16_SCR_PAL";
		case ZQKEY_GRID: return "ZQKEY_GRID";
		case ZQKEY_GRID_COLOR: return "ZQKEY_GRID_COLOR";
		case ZQKEY_COMBO_COL_MODE: return "ZQKEY_COMBO_COL_MODE";
		case ZQKEY_DELETE: return "ZQKEY_DELETE";
		case ZQKEY_FULLSCREEN: return "ZQKEY_FULLSCREEN";
		case ZQKEY_LYR_0: return "ZQKEY_LYR_0";
		case ZQKEY_LYR_1: return "ZQKEY_LYR_1";
		case ZQKEY_LYR_2: return "ZQKEY_LYR_2";
		case ZQKEY_LYR_3: return "ZQKEY_LYR_3";
		case ZQKEY_LYR_4: return "ZQKEY_LYR_4";
		case ZQKEY_LYR_5: return "ZQKEY_LYR_5";
		case ZQKEY_LYR_6: return "ZQKEY_LYR_6";
		case ZQKEY_SCR_LPAL_0: return "ZQKEY_SCR_LPAL_0";
		case ZQKEY_SCR_LPAL_1: return "ZQKEY_SCR_LPAL_1";
		case ZQKEY_SCR_LPAL_2: return "ZQKEY_SCR_LPAL_2";
		case ZQKEY_SCR_LPAL_3: return "ZQKEY_SCR_LPAL_3";
		case ZQKEY_SCR_LPAL_4: return "ZQKEY_SCR_LPAL_4";
		case ZQKEY_SCR_LPAL_5: return "ZQKEY_SCR_LPAL_5";
		case ZQKEY_SCR_LPAL_6: return "ZQKEY_SCR_LPAL_6";
		case ZQKEY_SCR_LPAL_7: return "ZQKEY_SCR_LPAL_7";
		case ZQKEY_SCR_LPAL_8: return "ZQKEY_SCR_LPAL_8";
		case ZQKEY_SCR_LPAL_9: return "ZQKEY_SCR_LPAL_9";
		case ZQKEY_SCR_LPAL_10: return "ZQKEY_SCR_LPAL_10";
		case ZQKEY_SCR_LPAL_11: return "ZQKEY_SCR_LPAL_11";
		case ZQKEY_SCR_LPAL_12: return "ZQKEY_SCR_LPAL_12";
		case ZQKEY_SCR_LPAL_13: return "ZQKEY_SCR_LPAL_13";
		case ZQKEY_SCR_LPAL_14: return "ZQKEY_SCR_LPAL_14";
		case ZQKEY_SCR_LPAL_15: return "ZQKEY_SCR_LPAL_15";
		case ZQKEY_SCROLL_SCREEN_UP: return "ZQKEY_SCROLL_SCREEN_UP";
		case ZQKEY_SCROLL_SCREEN_DOWN: return "ZQKEY_SCROLL_SCREEN_DOWN";
		case ZQKEY_SCROLL_SCREEN_LEFT: return "ZQKEY_SCROLL_SCREEN_LEFT";
		case ZQKEY_SCROLL_SCREEN_RIGHT: return "ZQKEY_SCROLL_SCREEN_RIGHT";
		case ZQKEY_WARP_SCREEN_UP: return "ZQKEY_WARP_SCREEN_UP";
		case ZQKEY_WARP_SCREEN_DOWN: return "ZQKEY_WARP_SCREEN_DOWN";
		case ZQKEY_WARP_SCREEN_LEFT: return "ZQKEY_WARP_SCREEN_LEFT";
		case ZQKEY_WARP_SCREEN_RIGHT: return "ZQKEY_WARP_SCREEN_RIGHT";
		case ZQKEY_SCROLL_COMBO_UP: return "ZQKEY_SCROLL_COMBO_UP";
		case ZQKEY_SCROLL_COMBO_DOWN: return "ZQKEY_SCROLL_COMBO_DOWN";
		case ZQKEY_SCROLL_COMBO_LEFT: return "ZQKEY_SCROLL_COMBO_LEFT";
		case ZQKEY_SCROLL_COMBO_RIGHT: return "ZQKEY_SCROLL_COMBO_RIGHT";
		case ZQKEY_COMBO_PAGEUP: return "ZQKEY_COMBO_PAGEUP";
		case ZQKEY_COMBO_PAGEDN: return "ZQKEY_COMBO_PAGEDN";
		case ZQKEY_SQUAREPANEL_UP: return "ZQKEY_SQUAREPANEL_UP";
		case ZQKEY_SQUAREPANEL_DOWN: return "ZQKEY_SQUAREPANEL_DOWN";
		case ZQKEY_TESTMODE: return "ZQKEY_TESTMODE";
		case ZQKEY_CAUTO_HEIGHTMINUS: return "ZQKEY_CAUTO_HEIGHTMINUS";
		case ZQKEY_CAUTO_HEIGHTPLUS: return "ZQKEY_CAUTO_HEIGHTPLUS";
		case ZQKEY_CURR_LAYER_HL: return "ZQKEY_CURR_LAYER_HL";
	}
	return "ZQ_NIL_KEY";
}

Hotkey zq_hotkeys[ZQKEY_MAX];

bool is_reserved_key(int c)
{
	switch(c)
	{
		case KEY_ESC: case KEY_F1:
			return true;
	}
	return false;
}
bool is_reserved_keycombo(int c, int modflag)
{
	if(c==KEY_F4 && (modflag&KB_ALT_FLAG))
		return true;
	return false;
}

void default_hotkeys()
{
	zq_hotkeys[ZQKEY_UNDO].setval(KEY_Z,KB_CTRL_FLAG, KEY_U,0);
	zq_hotkeys[ZQKEY_REDO].setval(KEY_Z,KB_CTRL_FLAG|KB_SHIFT_FLAG, KEY_Y,KB_CTRL_FLAG);
	zq_hotkeys[ZQKEY_PLUS_FLAG].setval(KEY_SLASH_PAD,0,KEY_CLOSEBRACE,0);
	zq_hotkeys[ZQKEY_MINUS_FLAG].setval(KEY_ASTERISK,0,KEY_OPENBRACE,0);
	zq_hotkeys[ZQKEY_SAVE].setval(KEY_F2,0,KEY_S,KB_CTRL_FLAG);
	zq_hotkeys[ZQKEY_SAVEAS].setval(0,0,0,0);
	zq_hotkeys[ZQKEY_OPEN].setval(KEY_F3,0,KEY_O,KB_CTRL_FLAG);
	zq_hotkeys[ZQKEY_SCREEN_PAL].setval(KEY_F4,0,0,0);
	zq_hotkeys[ZQKEY_SECRET_COMBO].setval(KEY_F5,0,0,0);
	zq_hotkeys[ZQKEY_DOORS].setval(KEY_F6,0,0,0);
	zq_hotkeys[ZQKEY_FFCS].setval(KEY_F7,0,0,0);
	zq_hotkeys[ZQKEY_FLAGS].setval(KEY_F8,0,0,0);
	zq_hotkeys[ZQKEY_SCRDATA].setval(KEY_F9,0,0,0);
	zq_hotkeys[ZQKEY_TILEWARP].setval(KEY_F10,0,0,0);
	zq_hotkeys[ZQKEY_SIDEWARP].setval(KEY_F11,0,0,0);
	zq_hotkeys[ZQKEY_LAYERS].setval(KEY_F12,0,0,0);
	zq_hotkeys[ZQKEY_RESET_TRANSP].setval(KEY_B,0,0,0);
	zq_hotkeys[ZQKEY_COPY].setval(KEY_C,0,0,0);
	zq_hotkeys[ZQKEY_TOGGLE_DARK].setval(KEY_D,0,0,0);
	zq_hotkeys[ZQKEY_ENEMIES].setval(KEY_E,0,0,0);
	zq_hotkeys[ZQKEY_SHOW_FLAGS].setval(KEY_F,0,0,0);
	zq_hotkeys[ZQKEY_ITEMS].setval(KEY_I,0,0,0);
	zq_hotkeys[ZQKEY_COMBOS].setval(KEY_K,0,0,0);
	zq_hotkeys[ZQKEY_DARK_PREVIEW].setval(KEY_L,0,0,0);
	zq_hotkeys[ZQKEY_SHOW_INFO].setval(KEY_N,0,0,0);
	zq_hotkeys[ZQKEY_DRAWING_MODE].setval(KEY_O,0,0,0);
	zq_hotkeys[ZQKEY_GOTO_PAGE].setval(KEY_P,0,0,0);
	zq_hotkeys[ZQKEY_SHOW_CMB_CS_INFO].setval(KEY_Q,0,0,0);
	zq_hotkeys[ZQKEY_ROOM].setval(KEY_R,0,0,0);
	zq_hotkeys[ZQKEY_STRINGS].setval(KEY_S,0,0,0);
	zq_hotkeys[ZQKEY_TILES].setval(KEY_T,0,0,0);
	zq_hotkeys[ZQKEY_PASTE].setval(KEY_V,0,0,0);
	zq_hotkeys[ZQKEY_PASTEALL].setval(KEY_V,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_PASTETOALL].setval(KEY_V,KB_CTRL_FLAG,0,0);
	zq_hotkeys[ZQKEY_PASTEALLTOALL].setval(KEY_V,KB_CTRL_FLAG|KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_SHOW_SOLID].setval(KEY_W,0,0,0);
	zq_hotkeys[ZQKEY_PREV_MODE].setval(KEY_X,0,0,0);
	zq_hotkeys[ZQKEY_COMPILE_ZSCRIPT].setval(KEY_Y,0,0,0);
	zq_hotkeys[ZQKEY_SCREENSHOT].setval(KEY_Z,0,0,0);
	zq_hotkeys[ZQKEY_ABOUT].setval(KEY_SLASH,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_MINUS_MAP].setval(KEY_COMMA,0,0,0);
	zq_hotkeys[ZQKEY_PLUS_MAP].setval(KEY_STOP,0,0,0);
	zq_hotkeys[ZQKEY_MINUS_COLOR].setval(KEY_MINUS,0,KEY_MINUS_PAD,0);
	zq_hotkeys[ZQKEY_PLUS_COLOR].setval(KEY_EQUALS,0,KEY_PLUS_PAD,0);
	zq_hotkeys[ZQKEY_MINUS_SCR_PAL].setval(KEY_MINUS,KB_SHIFT_FLAG,KEY_MINUS_PAD,KB_SHIFT_FLAG);
	zq_hotkeys[ZQKEY_PLUS_SCR_PAL].setval(KEY_EQUALS,KB_SHIFT_FLAG,KEY_PLUS_PAD,KB_SHIFT_FLAG);
	zq_hotkeys[ZQKEY_MINUS_16_SCR_PAL].setval(KEY_MINUS,KB_CTRL_FLAG,KEY_MINUS_PAD,KB_CTRL_FLAG);
	zq_hotkeys[ZQKEY_PLUS_16_SCR_PAL].setval(KEY_EQUALS,KB_CTRL_FLAG,KEY_PLUS_PAD,KB_CTRL_FLAG);
	zq_hotkeys[ZQKEY_GRID].setval(KEY_TILDE,0,0,0);
	zq_hotkeys[ZQKEY_GRID_COLOR].setval(KEY_TILDE,KB_CTRL_FLAG,0,0);
	zq_hotkeys[ZQKEY_COMBO_COL_MODE].setval(KEY_SPACE,0,0,0);
	zq_hotkeys[ZQKEY_DELETE].setval(KEY_DEL,0,KEY_DEL_PAD,0);
	zq_hotkeys[ZQKEY_FULLSCREEN].setval(KEY_ENTER,KB_ALT_FLAG,KEY_ENTER_PAD,KB_ALT_FLAG);
	for(int q = 0; q <= 6; ++q) //0-6, +pad
		zq_hotkeys[ZQKEY_LYR_0+q].setval(KEY_0+q,0,KEY_0_PAD+q,0);
	for(int q = 0; q <= 15; ++q)
	{
		if(q < 10) //Ctrl+(0-9 + pad), Ctrl+Shift+(0-5 + pad)
			zq_hotkeys[ZQKEY_SCR_LPAL_0+q].setval(KEY_0+q,KB_CTRL_FLAG,KEY_0_PAD+q,KB_CTRL_FLAG);
		else zq_hotkeys[ZQKEY_SCR_LPAL_0+q].setval(KEY_0+(q-10),KB_CTRL_FLAG|KB_SHIFT_FLAG,
			KEY_0_PAD+(q-10),KB_CTRL_FLAG|KB_SHIFT_FLAG);
	}
	zq_hotkeys[ZQKEY_SCROLL_SCREEN_UP].setval(KEY_UP,0,0,0);
	zq_hotkeys[ZQKEY_SCROLL_SCREEN_DOWN].setval(KEY_DOWN,0,0,0);
	zq_hotkeys[ZQKEY_SCROLL_SCREEN_LEFT].setval(KEY_LEFT,0,0,0);
	zq_hotkeys[ZQKEY_SCROLL_SCREEN_RIGHT].setval(KEY_RIGHT,0,0,0);
	zq_hotkeys[ZQKEY_WARP_SCREEN_UP].setval(KEY_UP,KB_CTRL_FLAG,0,0);
	zq_hotkeys[ZQKEY_WARP_SCREEN_DOWN].setval(KEY_DOWN,KB_CTRL_FLAG,0,0);
	zq_hotkeys[ZQKEY_WARP_SCREEN_LEFT].setval(KEY_LEFT,KB_CTRL_FLAG,0,0);
	zq_hotkeys[ZQKEY_WARP_SCREEN_RIGHT].setval(KEY_RIGHT,KB_CTRL_FLAG,0,0);
	zq_hotkeys[ZQKEY_SCROLL_COMBO_UP].setval(KEY_UP,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_SCROLL_COMBO_DOWN].setval(KEY_DOWN,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_SCROLL_COMBO_LEFT].setval(KEY_LEFT,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_SCROLL_COMBO_RIGHT].setval(KEY_RIGHT,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_COMBO_PAGEUP].setval(KEY_PGUP,0,0,0);
	zq_hotkeys[ZQKEY_COMBO_PAGEDN].setval(KEY_PGDN,0,0,0);
	zq_hotkeys[ZQKEY_SQUAREPANEL_UP].setval(KEY_PGUP,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_SQUAREPANEL_DOWN].setval(KEY_PGDN,KB_SHIFT_FLAG,0,0);
	zq_hotkeys[ZQKEY_TESTMODE].setval(KEY_T,KB_CTRL_FLAG,0,0);
	zq_hotkeys[ZQKEY_CAUTO_HEIGHTPLUS].setval(KEY_CLOSEBRACE, KB_SHIFT_FLAG, 0, 0);
	zq_hotkeys[ZQKEY_CAUTO_HEIGHTMINUS].setval(KEY_OPENBRACE, KB_SHIFT_FLAG, 0, 0);
	zq_hotkeys[ZQKEY_CURR_LAYER_HL].setval(0, 0, 0, 0);
}

void load_hotkeys()
{
	default_hotkeys();
	for(int q = 0; q < ZQKEY_MAX; ++q)
	{
		int v = zq_hotkeys[q].getval();
		int nv = zc_get_config("ZQ_HOTKEY",get_hotkey_cfg_name(q),v);
		if(v!=nv)
			zq_hotkeys[q].setval(nv);
	}
}

int run_hotkey(int hkey)
{
	switch(hkey)
	{
		case ZQKEY_UNDO:
			return onUndo();
		case ZQKEY_REDO:
			return onRedo();
		case ZQKEY_PLUS_FLAG:
			return onIncreaseFlag();
		case ZQKEY_MINUS_FLAG:
			return onDecreaseFlag();
		case ZQKEY_SAVE:
			if(saved|disable_saving|OverwriteProtection)
				break;
			return onSave();
		case ZQKEY_SAVEAS:
			return onSaveAs();
		case ZQKEY_OPEN:
			return do_OpenQuest();
		case ZQKEY_SCREEN_PAL:
			return onScreenPalette();
		case ZQKEY_SECRET_COMBO:
			return onSecretCombo();
		case ZQKEY_DOORS:
			return onDoors();
		case ZQKEY_FFCS:
			return onSelectFFCombo();
		case ZQKEY_FLAGS:
			return onFlags();
		case ZQKEY_SCRDATA:
			return onScrData();
		case ZQKEY_TILEWARP:
			return onTileWarp();
		case ZQKEY_SIDEWARP:
			return onSideWarp();
		case ZQKEY_LAYERS:
			return onLayers();
		case ZQKEY_RESET_TRANSP:
			return onResetTransparency();
		case ZQKEY_COPY:
			return onCopy();
		case ZQKEY_TOGGLE_DARK:
			return onToggleDarkness();
		case ZQKEY_ENEMIES:
			return onEnemies();
		case ZQKEY_SHOW_FLAGS:
			return onShowFlags();
		case ZQKEY_ITEMS:
			return onItem();
		case ZQKEY_COMBOS:
			return onCombos();
		case ZQKEY_DARK_PREVIEW:
			return onShowDarkness();
		case ZQKEY_SHOW_INFO:
			return onToggleShowInfo();
		case ZQKEY_DRAWING_MODE:
			return onDrawingMode();
		case ZQKEY_GOTO_PAGE:
			return onGotoPage();
		case ZQKEY_SHOW_CMB_CS_INFO:
			return onShowComboInfoCSet();
		case ZQKEY_ROOM:
			return onRoom();
		case ZQKEY_STRINGS:
			return onStrings();
		case ZQKEY_TILES:
			return onTiles();
		case ZQKEY_PASTE:
			return onPaste();
		case ZQKEY_PASTEALL:
			return onPasteAll();
		case ZQKEY_PASTETOALL:
			return onPasteToAll();
		case ZQKEY_PASTEALLTOALL:
			return onPasteAllToAll();
		case ZQKEY_SHOW_SOLID:
			return onShowWalkability();
		case ZQKEY_PREV_MODE:
			return onPreviewMode();
		case ZQKEY_COMPILE_ZSCRIPT:
			return onCompileScript();
		case ZQKEY_SCREENSHOT:
			return onSnapshot();
		case ZQKEY_ABOUT:
			return onAbout();
		case ZQKEY_MINUS_MAP:
			return onDecMap();
		case ZQKEY_PLUS_MAP:
			return onIncMap();
		case ZQKEY_MINUS_COLOR:
			return onDecreaseCSet();
		case ZQKEY_PLUS_COLOR:
			return onIncreaseCSet();
		case ZQKEY_MINUS_SCR_PAL:
			return onDecScrPal();
		case ZQKEY_PLUS_SCR_PAL:
			return onIncScrPal();
		case ZQKEY_MINUS_16_SCR_PAL:
			return onDecScrPal16();
		case ZQKEY_PLUS_16_SCR_PAL:
			return onIncScrPal16();
		case ZQKEY_GRID:
			return onToggleGrid(false);
		case ZQKEY_GRID_COLOR:
			return onToggleGrid(true);
			
		case ZQKEY_COMBO_COL_MODE:
			return onSpacebar();
		case ZQKEY_DELETE:
			return onDelete();
		case ZQKEY_FULLSCREEN:
			return onFullScreen();
		case ZQKEY_LYR_0: case ZQKEY_LYR_1: case ZQKEY_LYR_2: case ZQKEY_LYR_3:
		case ZQKEY_LYR_4: case ZQKEY_LYR_5: case ZQKEY_LYR_6:
			return onSetNewLayer(hkey-ZQKEY_LYR_0);
		case ZQKEY_SCR_LPAL_0: case ZQKEY_SCR_LPAL_1: case ZQKEY_SCR_LPAL_2: case ZQKEY_SCR_LPAL_3:
		case ZQKEY_SCR_LPAL_4: case ZQKEY_SCR_LPAL_5: case ZQKEY_SCR_LPAL_6: case ZQKEY_SCR_LPAL_7:
		case ZQKEY_SCR_LPAL_8: case ZQKEY_SCR_LPAL_9: case ZQKEY_SCR_LPAL_10: case ZQKEY_SCR_LPAL_11:
		case ZQKEY_SCR_LPAL_12: case ZQKEY_SCR_LPAL_13: case ZQKEY_SCR_LPAL_14: case ZQKEY_SCR_LPAL_15:
			return onScreenLPal(hkey-ZQKEY_SCR_LPAL_0);
		case ZQKEY_SCROLL_SCREEN_UP: case ZQKEY_SCROLL_SCREEN_DOWN:
		case ZQKEY_SCROLL_SCREEN_LEFT: case ZQKEY_SCROLL_SCREEN_RIGHT:
			return onScrollScreen(hkey-ZQKEY_SCROLL_SCREEN_UP,false);
		case ZQKEY_WARP_SCREEN_UP: case ZQKEY_WARP_SCREEN_DOWN:
		case ZQKEY_WARP_SCREEN_LEFT: case ZQKEY_WARP_SCREEN_RIGHT:
			return onScrollScreen(hkey-ZQKEY_WARP_SCREEN_UP,true);
		case ZQKEY_SCROLL_COMBO_UP:
			return onComboColUp();
		case ZQKEY_SCROLL_COMBO_DOWN:
			return onComboColDown();
		case ZQKEY_SCROLL_COMBO_LEFT:
			return onComboColLeft();
		case ZQKEY_SCROLL_COMBO_RIGHT:
			return onComboColRight();
		case ZQKEY_COMBO_PAGEUP:
			return onPgUp();
		case ZQKEY_COMBO_PAGEDN:
			return onPgDn();
		case ZQKEY_SQUAREPANEL_UP:
			cycle_compact_sqr(false);
			break;
		case ZQKEY_SQUAREPANEL_DOWN:
			cycle_compact_sqr(true);
			break;
		case ZQKEY_TESTMODE:
			call_testqst_dialog();
			break;
		case ZQKEY_CAUTO_HEIGHTMINUS:
			change_autocombo_height(-1);
			break;
		case ZQKEY_CURR_LAYER_HL:
			onToggleHighlightLayer();
			break;
	}
	return D_O_K;
}

int d_zq_hotkey_proc(int msg, DIALOG* d, int c)
{
	int ret = D_O_K;
	switch(msg)
	{
		case MSG_START:
			d->w = d->h = 0;
			break;
		case MSG_XCHAR:
			int key = c>>8;
			if(is_modkey(key) || !(key))
				break;
			int shifts = get_mods();
			if(key==KEY_F4 && (shifts&KB_ALT_FLAG))
			{
				close_button_quit = true;
				return D_USED_CHAR;
			}
			for(int pass = 0; pass <= 1; ++pass)
				for(int q = 0; q < ZQKEY_MAX; ++q)
				{
					if(zq_hotkeys[q].check(c>>8,shifts,!pass))
						return run_hotkey(q) | D_USED_CHAR;
				}
			break;
	}
	return ret;
}

int do_zq_hotkey_dialog()
{
	Hotkey tmp_hotkeys[ZQKEY_MAX];
	memcpy(tmp_hotkeys, zq_hotkeys, sizeof(tmp_hotkeys));
	
	bool confirm = false;
	ZQHotkeyDialog(&confirm).show();
	if(confirm)
	{
		for(int q = 0; q < ZQKEY_MAX; ++q)
		{
			if(tmp_hotkeys[q] != zq_hotkeys[q])
				zc_set_config("ZQ_HOTKEY",get_hotkey_cfg_name(q),zq_hotkeys[q].getval());
		}
	}
	else
		memcpy(zq_hotkeys, tmp_hotkeys, sizeof(tmp_hotkeys));
	return D_O_K;
}
