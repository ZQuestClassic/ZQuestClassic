#pragma once

#include <gui/dialog.h>
#include <gui/window.h>
#include <functional>

class GlobalInitDDialog: public GUI::Dialog<GlobalInitDDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK
	};

	GlobalInitDDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	std::shared_ptr<GUI::Window> window;
	
	// GUI::ListData list_;
};

