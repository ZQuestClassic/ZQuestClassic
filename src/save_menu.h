#pragma once

#include "base/headers.h"
#include "base/zc_alleg.h"
#include "base/flags.h"
#include "sfx.h"
#include <optional>

namespace flags {
enum save_menu_flags : uint16_t
{
	SMENU_CONFIRM_A       = F(0),
	SMENU_CANCEL_B        = F(1),
	SMENU_DONT_KILL_MUSIC = F(2),
	SMENU_REPEAT_BG       = F(3),
	SMENU_DONT_KILL_SFX = F(4),
};
enum save_menu_option_flags : uint16_t
{
	SMENU_OPT_EXIT        = F(0),
	SMENU_OPT_CONTINUE    = F(1),
	SMENU_OPT_RELOAD      = F(2),
	SMENU_OPT_SAVE        = F(3),
	SMENU_OPT_CANCEL      = F(4),
};
} // ends namespace
#define SMENU_FLAG_COUNT 3
struct SaveMenuOption
{
	string text;
	save_menu_option_flags flags;
	byte color = WHITE, picked_color = WHITE;
	int32_t font;
	word gen_script;
	bool operator==(SaveMenuOption const& other) const = default;
};

#define MAX_SAVEMENU_OPTIONS 20
#define SAVEMENU_NAME_LENGTH 32
#define SAVEMENU_STRING_LENGTH 48

struct SaveMenu
{
	string name;
	save_menu_flags flags;
	int32_t cursor_tile, bg_tile;
	byte cursor_cset, bg_cset, bg_tw = 1, bg_th = 1;
	byte bg_color = BLACK;
	word cursor_sfx = WAV_CHINK, choose_sfx = WAV_CHINK;
	byte hspace = 6, vspace = 16;
	byte opt_x = 128, opt_y = 32;
	byte text_align = ALIGN_LEFT, textbox_align = ALIGN_CENTER;
	word close_frames = 30;
	byte close_flash_rate = 5;
	word music;
	std::vector<SaveMenuOption> options;
	
	bool is_empty() const;
	bool is_valid() const;
	void clear();
	optional<byte> run(optional<byte> cursor = nullopt) const;
	bool tick(optional<byte>& cursor, word& clk) const;
	void draw(BITMAP* dest, optional<byte> cursor, word clk) const;
	bool operator==(SaveMenu const& other) const = default;
};
