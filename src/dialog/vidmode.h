#ifndef ZC_DIALOG_VIDMODE_H_
#define ZC_DIALOG_VIDMODE_H_

#include <gui/dialog.h>
#include <gui/checkbox.h>
#include <gui/text_field.h>
#include <initializer_list>
#include <string>
#include <string_view>

void call_vidmode_dlg();

// A basic dialog that just shows some lines of text and a close button.
class VidModeDialog: public GUI::Dialog<VidModeDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	VidModeDialog();

	std::shared_ptr<GUI::Widget> view() override;
	virtual bool handleMessage(const GUI::DialogMessage<message>& msg);
protected:
	std::shared_ptr<GUI::TextField> rx,ry;
};

#endif
