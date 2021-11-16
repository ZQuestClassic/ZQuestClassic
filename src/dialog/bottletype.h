#ifndef ZC_DIALOG_ROOM_H
#define ZC_DIALOG_ROOM_H

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>

class BottleTypeDialog: public GUI::Dialog<BottleTypeDialog>
{
public:
	enum class message
	{
		OK, CANCEL
	};

	BottleTypeDialog::BottleTypeDialog(int32_t index);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	int32_t index;
	bottletype& sourceBottle;
	bottletype tempBottle;
	GUI::ListData list_counters;
};

#endif
