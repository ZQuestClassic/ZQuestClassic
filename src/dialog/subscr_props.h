#ifndef ZC_DIALOG_SUBSCR_PROPS_H
#define ZC_DIALOG_SUBSCR_PROPS_H

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <gui/window.h>
#include <initializer_list>
#include <string>
#include <string_view>
#include "subscr.h"

// A basic dialog that just shows some lines of text and a close button.
class SubscrPropDialog: public GUI::Dialog<SubscrPropDialog>
{
public:
	enum class message
	{
		OK, CANCEL
	};

	SubscrPropDialog(subscreen_object *ref, int32_t obj_ind);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

protected:
	std::shared_ptr<GUI::Window> window;
	subscreen_object *subref;
	subscreen_object local_subref;
	int32_t index;
};

#endif
