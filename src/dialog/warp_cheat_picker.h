#pragma once

#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/label.h>
#include <functional>

class WarpCheatPicker: public GUI::Dialog<WarpCheatPicker>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	WarpCheatPicker();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> info_label;
	GUI::ListData list_dmaps;
	int dest_dmap, dest_screen, adjusted_screen;
	
	void refresh_label();
	void refresh_screen();
};

