#pragma once

#include <gui/dialog.h>
#include <gui/window.h>
#include <gui/label.h>
#include <functional>

class VolumeDialog: public GUI::Dialog<VolumeDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	VolumeDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Label> vol_labels[3];
};

