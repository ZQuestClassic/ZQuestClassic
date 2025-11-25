#pragma once

#include <gui/dialog.h>
#include <gui/window.h>
#include <functional>
#include "base/headers.h"
#include "base/mapscr.h"

void call_undercombo_dlg(int map, int screen);

class UnderComboDialog : public GUI::Dialog<UnderComboDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	UnderComboDialog(int map, int screen);

	shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	void refresh_layer(int layer);
	bool save_results();
	shared_ptr<GUI::Window> window;
	int map, screen;
	
	mapscr* cur_screens[7];
	bool layer_enabled[7];
	word undercombos[7];
	byte undercsets[7];
	
	std::array<vector<shared_ptr<GUI::Widget>>, 7> lyr_widgets;
	
	size_t preview_idx, underc_idx;
	
	bool overwrite_mode, whole_map;
};

