#ifndef ZC_DIALOG_ROOM_H
#define ZC_DIALOG_ROOM_H

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/label.h>
#include <gui/switcher.h>
#include <gui/text_field.h>
#include <functional>

class BottleTypeDialog: public GUI::Dialog<BottleTypeDialog>
{
public:
	enum class message
	{
		OK, CANCEL
	};

	BottleTypeDialog::BottleTypeDialog(bottletype& theBottle);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	bottletype& sourcebottle;
	bottletype tempbottle;
};

#endif
