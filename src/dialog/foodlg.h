#ifndef ZC_DIALOG_FOO_H_
#define ZC_DIALOG_FOO_H_

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

void call_foo_dlg();

class FooDialog: public GUI::Dialog<FooDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	FooDialog(int32_t v, std::function<void(int32_t,bool)> setVal);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::TextField> datafield;
	std::function<void(int32_t,bool)> setVal;
	int32_t v;
};

#endif
