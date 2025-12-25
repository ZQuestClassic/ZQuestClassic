#pragma once

#include <gui/dialog.h>
#include <gui/window.h>
#include <functional>

class ScreenSaverDialog: public GUI::Dialog<ScreenSaverDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL, PREVIEW
	};

	ScreenSaverDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	std::shared_ptr<GUI::Window> window;
	int speed, density, seconds;
	bool enabled;
};

