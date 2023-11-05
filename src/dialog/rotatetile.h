#ifndef ZC_DIALOG_ROTATETILE_H
#define ZC_DIALOG_ROTATETILE_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

class RotateTileDialog : public GUI::Dialog<RotateTileDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	RotateTileDialog(int32_t* w, int32_t* h, zfix* r);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	int32_t swid, shei;
	zfix tr;
	zfix* trot;
};

#endif
