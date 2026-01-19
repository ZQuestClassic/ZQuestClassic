#include "save_menu.h"
#include "base/fonts.h"
#include "tiles.h"
#include "zc/zc_sys.h"
#include "advanced_music.h"

#ifdef IS_PLAYER
#include "zc/zelda.h"
#include "zc/title.h"
#include "zc/ffscript.h"
extern int32_t skipcont;
void playLevelMusic();
#else
#include "gui/jwin.h"
#include "zq/zquest.h"
#include "zq/render.h"
#endif

bool SaveMenu::is_empty() const
{
	return *this == SaveMenu();
}
bool SaveMenu::is_valid() const
{
	return !options.empty();
}

void SaveMenu::clear()
{
	*this = SaveMenu();
}

optional<byte> SaveMenu::run(optional<byte> cursor) const
{
	word clk = 0;
#ifdef IS_PLAYER
	bool is_dead = Quit == qGAMEOVER;
	Quit = 0;
	
	bool use_music = !(flags & SMENU_DONT_KILL_MUSIC);
	bool stop_sounds = !(flags & SMENU_DONT_KILL_SFX);
	
	if (stop_sounds)
	{
		map_bkgsfx(false);
		kill_sfx();
	}
	
	auto was_playing = Playing;
	Playing = false;
	bool running = true;
	while (running)
	{
		if (use_music && unsigned(music-1) < quest_music.size())
			quest_music[music-1].play();
		while (!tick(cursor, clk))
		{
			if (Quit)
			{
				cursor = nullopt;
				running = false;
				break;
			}
			draw(framebuf, cursor, clk);
			advanceframe(false, true, false);
			load_control_state();
		}
		
		if (cursor)
		{
			SaveMenuOption const& opt = options[*cursor];
			
			if (opt.flags & SMENU_OPT_SAVE)
				save_game(false);
			
			if (opt.gen_script)
				FFCore.runGenericFrozenEngine(opt.gen_script);
			
			running = false;
			if (Quit)
				;
			else if (opt.flags & SMENU_OPT_RELOAD)
				Quit = qRELOAD;
			else if (opt.flags & SMENU_OPT_EXIT)
				Quit = qQUIT;
			else if (opt.flags & SMENU_OPT_CONTINUE)
				Quit = qCONT;
			else if (!is_dead && (opt.flags & SMENU_OPT_CANCEL))
				Quit = 0;
			else running = true;
		}
		else if (Quit)
			running = false;
		else // canceled with 'B'
		{
			if (!is_dead)
			{
				Quit = 0;
				running = false;
			}
		}
	}
	if (Quit)
		skipcont = true;
	else
	{
		if (use_music)
			playLevelMusic();
		if (stop_sounds)
			map_bkgsfx(true);
	}
	Playing = was_playing;
#else
	const int scale = 2;
	const int w = 256, h = 236;
	const int sw = w * scale, sh = h * scale;
	BITMAP* tmp_bmp = create_bitmap_ex(8, w, h);
	
	FONT* f = get_custom_font(CFONT_DLG);
	auto prev_x = (zq_screen_w - sw) / 2;
	auto prev_y = (zq_screen_h - sh) / 2;
	const int lbl_margin = 2;
	auto lbl_hei = 2*lbl_margin+text_height(f);
	const int win_w = sw + 6 + 6;
	const int win_h = sh + lbl_hei + 25 + 6;
	const int win_xoff = -6;
	const int win_yoff = -25;
	popup_zqdialog_start(prev_x + win_xoff, prev_y + win_yoff, win_w, win_h);
	clear_to_color(screen, jwin_pal[jcBOX]);
	
	jwin_draw_win(screen, 0, 0, win_w, win_h, FR_WIN);
	jwin_draw_frame(screen, 4, 23, win_w - 8, win_h - 23 - 4 - lbl_hei,  FR_DEEP);
	
	FONT *oldfont = font;
	font = get_zc_font(font_lfont);
	jwin_draw_titlebar(screen, 3, 3, win_w - 6, 18, fmt::format("Preview: Save Menu '{}'", name).c_str(), false);
	font = oldfont;
	
	textout_centre_ex(screen, f, "ENTER = Select, ESC = Cancel, UP/DOWN = Change Selection", win_w / 2, -win_yoff+sh+lbl_margin+4-2, jwin_pal[jcBOXFG], -1);
	
	popup_zqdialog_start(prev_x, prev_y, sw, sh);
	zqdialog_set_skiptint(true);
	zq_push_unfrozen_dialogs(2);
	kill_sfx();
	while (!tick(cursor, clk))
	{
		draw(tmp_bmp, cursor, clk);
		stretch_blit(tmp_bmp, screen, 0, 0, w, h, 0, 0, sw, sh);
		anim_hw_screen();
	}
	zq_pop_unfrozen_dialogs();
	popup_zqdialog_end();
	popup_zqdialog_end();
	destroy_bitmap(tmp_bmp);
#endif
	return cursor;
}

bool SaveMenu::tick(optional<byte>& cursor, word& clk) const
{
	if (options.empty())
		return true;
	if (clk) // no control during closing animation
		return !--clk;
#ifdef IS_PLAYER
	byte c = cursor ? *cursor : 0;
	
	if (getInput(btnUp, INPUT_PRESS))
	{
		c = c ? c - 1 : options.size() - 1;
		if (cursor_sfx)
			sfx(cursor_sfx);
	}
	if (getInput(btnDown, INPUT_PRESS))
	{
		c = c < options.size() - 1 ? c + 1 : 0;
		if (cursor_sfx)
			sfx(cursor_sfx);
	}
	
	cursor = c;
	
	bool confirm = getInput(btnS, INPUT_PRESS);
	bool cancel = false;
	if (flags & SMENU_CONFIRM_A)
		confirm = confirm || getInput(btnA, INPUT_PRESS);
	if (flags & SMENU_CANCEL_B)
		cancel = cancel || getInput(btnB, INPUT_PRESS);
	
	if (confirm)
	{
		clk = close_frames; // closing animation
		if (choose_sfx)
			sfx(choose_sfx);
	}
	else if (cancel)
	{
		cursor = nullopt;
		return true;
	}
#else
	byte c = cursor ? *cursor : 0;
	bool confirm = false;
	bool cancel = false;
	
	if (keypressed())
	{
		int k = readkey()>>8;
		switch (k)
		{
			case KEY_ENTER: case KEY_ENTER_PAD:
				confirm = true;
				break;
			case KEY_ESC:
				cancel = true;
				break;
			case KEY_UP:
				c = c ? c - 1 : options.size() - 1;
				if (cursor_sfx)
					sfx(cursor_sfx);
				break;
			case KEY_DOWN:
				c = c < options.size() - 1 ? c + 1 : 0;
				if (cursor_sfx)
					sfx(cursor_sfx);
				break;
		}
		clear_keybuf();
	}
	
	cursor = c;
	
	if (confirm)
	{
		clk = close_frames; // closing animation
		if (choose_sfx)
			sfx(choose_sfx);
	}
	else if (cancel)
	{
		cursor = nullopt;
		return true;
	}
#endif
	return false;
}

void SaveMenu::draw(BITMAP* dest, optional<byte> cursor, word clk) const
{
	const int TARG_WID = 256, TARG_HEI = 236;
	int clipx1, clipy1, clipx2, clipy2;
	get_clip_rect(dest, &clipx1, &clipy1, &clipx2, &clipy2);
	set_clip_rect(dest, 0, 0, TARG_WID, TARG_HEI);
	
#ifdef IS_PLAYER
	clear_to_color(dest, bg_color);
#else
	clear_to_color(dest, zq_fix_ui_color(bg_color));
#endif
	if (bg_tile)
	{
		if (flags & SMENU_REPEAT_BG)
		{
			for (int t_y = 0; t_y < TARG_HEI; t_y += 16*bg_th)
				for (int t_x = 0; t_x < TARG_WID; t_x += 16*bg_tw)
					overtileblock16(dest, bg_tile, t_x, t_y, bg_tw, bg_th, bg_cset, 0, 0);
		}
		else overtileblock16(dest, bg_tile, 0, 0, bg_tw, bg_th, bg_cset, 0, 0);
	}
	
	int y = opt_y;
	size_t ind = 0;
	int txbox_w = 0;
	for (auto const& opt : options)
	{
		FONT* optfont = get_zc_font(opt.font);
		auto w = text_length(optfont, opt.text.c_str());
		if (w > txbox_w)
			txbox_w = w;
	}
	int txbox_x = opt_x;
	switch (textbox_align)
	{
		case ALIGN_CENTER:
			txbox_x -= txbox_w / 2;
			break;
		case ALIGN_RIGHT:
			txbox_x -= txbox_w;
			break;
	}
	
	for (auto const& opt : options)
	{
		FONT* optfont = get_zc_font(opt.font);
		auto h = text_height(optfont);
		auto w = text_length(optfont, opt.text.c_str());
		auto x = txbox_x;
		switch (text_align)
		{
			case ALIGN_CENTER:
				x = (x + txbox_w / 2) - (w / 2);
				break;
			case ALIGN_RIGHT:
				x = (x + txbox_w) - w;
				break;
		}
		auto color = opt.color;
		if (cursor && ind == *cursor) // currently selected
		{
			if (!clk || !close_flash_rate || (clk % (close_flash_rate*2)) < close_flash_rate) // not flashing, or not currently flashed
				color = opt.picked_color;
			overtile16(dest, cursor_tile, x - 16 - hspace, y + (h/2) - 8, cursor_cset, 0);
		}
#ifndef IS_PLAYER
		color = zq_fix_ui_color(color);
#endif
		textout_ex(dest, optfont, opt.text.c_str(), x, y, color, -1);
		y += vspace + h;
		++ind;
	}
	set_clip_rect(dest, clipx1, clipy1, clipx2, clipy2);
}