#ifndef ZC_DIALOG_MISCCOLORS_EDITOR_H_
#define ZC_DIALOG_MISCCOLORS_EDITOR_H_

#include <gui/dialog.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>
#include <string_view>
#include <base/misctypes.h>

bool call_misccolor_dialog();

class MiscColorDialog : public GUI::Dialog<MiscColorDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	MiscColorDialog();
	byte color_buf[17];
	byte *si;
	string color_strings[17] = {
		"Text:",
		"Caption:",
		"Overworld Minmap:",
		"Minimap Background:",
		"Minimap Foreground 1:",
		"Minimap Foreground 2:",
		"BS Minimap Dark:",
		"BS Minimap Goal:",
		"Compass Mark (Light):",
		"Compass Mark (Dark):",
		"Subscreen Background:",
		"Subscreen Shadow:",
		"Triforce Frame:",
		"Big Map Background:",
		"Big Map Foreground:",
		"Player's Position:",
		"Message Text:",
	};

	std::shared_ptr<GUI::Window> window;
	friend bool call_misccolor_dialog();
};
#endif
