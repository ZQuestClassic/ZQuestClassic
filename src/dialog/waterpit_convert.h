#ifndef ZC_DIALOG_PitConvert_H
#define ZC_DIALOG_PitConvert_H

#include <gui/dialog.h>
#include "gui/radioset.h"
#include "gui/label.h"
#include <functional>
#include <string_view>

int32_t onPitFix();

class PitConvertDialog: public GUI::Dialog<PitConvertDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	PitConvertDialog();

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
};

#endif
