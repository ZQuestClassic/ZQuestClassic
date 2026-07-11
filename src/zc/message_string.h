#pragma once

#include "base/headers.h"
#include "base/general.h"
#include "core/mapscr.h"
#include "core/msgstr.h"

namespace msgstr
{
	extern mapscr* scr;
	extern word active_str;
	extern bool on_screen, active;
	
	extern byte layer;
	extern bool nofreeze;
	
	extern word clk, linked_clk;
	
	bool allocate_bmps();
	void cleanup_bmps();
	
	void clear_display_bmps();
	void clear_bmps();
	
	void blit_str(BITMAP* dest, int sx, int sy, int dx, int dy, int w, int h);
	
	void do_new(mapscr* scr, int str);
	
	void dismiss();
	
	bool show_more();
	std::pair<int,int> more_xy();
	
	MsgStr const& get_str();
	
	void queue_string(int str);
	
	void run();
	void run_intro();
}

