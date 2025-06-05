#ifndef ZC_DIALOG_DOOR_SEL_H_
#define ZC_DIALOG_DOOR_SEL_H_

#include <gui/dialog.h>
#include <gui/button.h>
#include <gui/label.h>
#include <gui/drop_down_list.h>
#include <gui/window.h>
#include <gui/list_data.h>

bool call_doorseldialog();

class DoorSelDialog : public GUI::Dialog<DoorSelDialog>
{
public:
	enum class message {
		OK, CANCEL, CLEAR
	};

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	DoorSelDialog();
	GUI::ListData list_doortypes, list_doorsets;

	mapscr* m;
	byte doors[4];
	word dcs;
	word old_dcs;

	std::shared_ptr<GUI::Window> window;
	std::shared_ptr<GUI::Widget> DoorDDL(byte direction);

	friend bool call_doorseldialog();
};
#endif