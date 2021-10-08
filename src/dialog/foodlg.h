#ifndef ZC_DIALOG_FOO_H
#define ZC_DIALOG_FOO_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

void call_foo_dlg();

class FooDialog: public GUI::Dialog<FooDialog>
{
public:
	enum class message { OK, CANCEL };

	FooDialog(int v, std::function<void(int,bool)> setVal);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	std::shared_ptr<GUI::TextField> datafield;
	std::function<void(int,bool)> setVal;
	int v;
};

#endif
