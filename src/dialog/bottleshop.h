#ifndef ZC_DIALOG_BOTTLESHOPTYPE_H_
#define ZC_DIALOG_BOTTLESHOPTYPE_H_

#include <gui/dialog.h>
#include <gui/drop_down_list.h>
#include <gui/checkbox.h>
#include <gui/label.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <functional>
#include "base/misctypes.h"

class BottleShopDialog: public GUI::Dialog<BottleShopDialog>
{
public:
	enum class message
	{
		REFR_INFO, OK, CANCEL
	};

	BottleShopDialog(int32_t index);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::Window> window;
	int32_t index;
	bottleshoptype& sourceBottleShop;
	bottleshoptype tempBottleShop;
	GUI::ListData list_bottletypes;
};

#endif
