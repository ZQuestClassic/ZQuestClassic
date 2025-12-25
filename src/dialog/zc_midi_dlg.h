#pragma once

#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/label.h>
#include <gui/button.h>
#include <functional>

class ZCMidiDlg: public GUI::Dialog<ZCMidiDlg>
{
public:
	enum class message
	{
		REFR_INFO, OK, LISTEN, SAVE
	};

	ZCMidiDlg();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> description;
	std::shared_ptr<GUI::Button> save_btn;
	
	int tune = 0;
	bool listening = false;
	
	GUI::ListData list_midis;
	
	void refresh_status();
};

