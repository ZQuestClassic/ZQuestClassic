#ifndef ZC_DIALOG_FOO_H
#define ZC_DIALOG_FOO_H

#include <gui/dialog.h>
#include <gui/text_field.h>
#include <functional>
#include <string_view>

class ScaleTileDialog: public GUI::Dialog<ScaleTileDialog>
{
public:
	enum class message { REFR_INFO, OK, CANCEL };

	ScaleTileDialog(int32_t *w, int32_t *h);

	std::shared_ptr<GUI::Widget> view() override;
	bool handleMessage(const GUI::DialogMessage<message>& msg);

private:
	int32_t swid, shei, tw, th;
	int32_t *twid, *thei;
};

#endif
