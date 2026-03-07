#pragma once

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/window.h>
#include <functional>

class EditorHelpDialog: public GUI::Dialog<EditorHelpDialog>
{
public:
	enum class message
	{
		REFR_INFO, CLOSE
	};

	EditorHelpDialog() = default;

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);
private:
	std::shared_ptr<GUI::Window> window;
};

